/*
 * Copyright (c) 2010 John Nesky
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Type.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/PassManager.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/StandardPasses.h>
#include <llvm/Target/TargetSelect.h>
#include <llvm/LinkAllPasses.h>

#include "LoopJIT.h"
#include "loop-data.h"

using namespace llvm;

static ExecutionEngine *EE = 0;
static MemoryBuffer *Buffer = 0;

void* RunLoopJIT(LoopOptions loopOptions) {
  // TODO: Decide which stuff only needs to be constructed once and make those static.
  // TODO: Make sure we're not leaking any LLVM stuff. 
  // TODO: Figure out where each of the three buffers is used at all, and if not, stop
  //       calculating the buffer's phase. 
  
  std::string ErrorMsg;
  if (Buffer == 0) {
    InitializeNativeTarget();
    Buffer = MemoryBuffer::getMemBuffer(loop_bitcode, loop_bitcode + sizeof(loop_bitcode) - 1);
  }
  LLVMContext &Context = getGlobalContext();
  
  ModuleProvider *MP = getBitcodeModuleProvider(Buffer, Context, &ErrorMsg);
  Module *Mod = MP->materializeModule(&ErrorMsg);
  
  if (EE == 0) {
    EngineBuilder builder(MP);
    builder.setErrorStr(&ErrorMsg);
    builder.setEngineKind(EngineKind::JIT);
    CodeGenOpt::Level OLvl = CodeGenOpt::Aggressive;
    builder.setOptLevel(OLvl);
    EE = builder.create();
    EE->DisableLazyCompilation();
  } else {
    EE->addModuleProvider(MP);
  }
  if (!EE) {
    if (!ErrorMsg.empty())
      errs() << "error creating EE: " << ErrorMsg << "\n";
    else
      errs() << "unknown error creating EE!\n";
    exit(1);
  }
  Constant* c;
  
  
  
  
  Function* oldLoop = Mod->getFunction("loop");
  
  
  Function::arg_iterator oldArgs = oldLoop->arg_begin();
  oldArgs++; // do not mirror the first argument.
  std::vector<const Type*> newArgTypes;
  for (; oldArgs != oldLoop->arg_end(); oldArgs++) {
    newArgTypes.push_back((*oldArgs).getType());
  }
  
  FunctionType* newFuncType = FunctionType::get(Type::getVoidTy(Context),
                                                newArgTypes,
                                                false); // has variable number of arguments? No.
  
  c = Mod->getOrInsertFunction("optimizedLoop", newFuncType);
  Function* optimizedLoop = cast<Function>(c);
  BasicBlock* block = BasicBlock::Create(Context, "entry", optimizedLoop);
  IRBuilder<> irbuilder(block);
  
  
  
  
  SmallVector<Value*, 10> passedParams;
  char options[NUM_EFFECT_TYPES + 1];
  for (int i = 0; i < NUM_EFFECT_TYPES; i++) {
    options[i] = loopOptions.options[i];
  }
  options[NUM_EFFECT_TYPES] = 0;
  
  StringRef stringRef(options, NUM_EFFECT_TYPES);
  Constant *StrConstant = ConstantArray::get(Context, stringRef, true);
  GlobalVariable *gv = new GlobalVariable(*Mod,
                                          StrConstant->getType(),
                                          true,
                                          GlobalValue::InternalLinkage,
                                          StrConstant,
                                          "",
                                          0,
                                          false);
  Value *zero = ConstantInt::get(Type::getInt32Ty(Context), 0);
  Value *Args[] = { zero, zero };
  Value* mystring = irbuilder.CreateInBoundsGEP(gv, Args, Args+2, "");
  Value* myStringPtr = irbuilder.CreateConstGEP1_32(mystring, 0);
  
  
  
  passedParams.push_back(myStringPtr);
  Function::arg_iterator newArgs = optimizedLoop->arg_begin();
  for (; newArgs != optimizedLoop->arg_end(); newArgs++) {
    passedParams.push_back(&(*newArgs));
  }
  irbuilder.CreateCall(oldLoop, passedParams.begin(), passedParams.end(), "");
  
  irbuilder.CreateRetVoid();
  
  
  
  // First do a simple inline, and then remove the original function before 
  // bothering with the rest of the optimizations. 
  
  oldLoop->addFnAttr(Attribute::AlwaysInline);
  PassManager inliner;
  inliner.add(createAlwaysInlinerPass());
  inliner.run(*Mod);
  oldLoop->dropAllReferences();
  Mod->getFunctionList().remove(oldLoop);
  
  
  // Optimize some more stuff for kicks:
  
  PassManager PM;
  PM.add(new TargetData(*EE->getTargetData()));
  PM.add(createLowerSetJmpPass());
  createStandardModulePasses(&PM, 4,
                             false, // OptimizeSize
                             true, // UnitAtATime
                             true, // UnrollLoops
                             true, // SimplifyLibCalls
                             false, // HaveExceptions
                             createFunctionInliningPass());
  PM.add(createStripSymbolsPass(true));
  createStandardLTOPasses(&PM, true, //Internalize
                               true, // RunInliner
                               false); // VerifyEach
  
  FunctionPassManager FPM(MP);
  FPM.add(new TargetData(*EE->getTargetData()));
  createStandardFunctionPasses(&FPM, 4);
  
  PM.run(*Mod);
  FPM.run(*optimizedLoop);
  
  //Mod->dump();
  printf("Ran LoopJIT\n");
  
  // Build and run:
  
  EE->runStaticConstructorsDestructors(Mod, false); // constructors
  //void *FPtr = EE->getPointerToFunction(oldLoop);
  //void (*FP)() = (void (*)())FPtr;
  //FP();
  //EE->runStaticConstructorsDestructors(Mod, true); // destructors
  
  // Just build:
  return EE->getPointerToFunction(optimizedLoop);
}
