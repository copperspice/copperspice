/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MacroAssemblerX86_64_h
#define MacroAssemblerX86_64_h

#include <wtf/Platform.h>

#if ENABLE(ASSEMBLER) && CPU(X86_64)

#include "MacroAssemblerX86Common.h"

#define REPTACH_OFFSET_CALL_R11 3

namespace JSC {

class MacroAssemblerX86_64 : public MacroAssemblerX86Common {
protected:
    static const X86Registers::RegisterID scratchRegister = X86Registers::r11;

public:
    static const Scale ScalePtr = TimesEight;

    using MacroAssemblerX86Common::add32;
    using MacroAssemblerX86Common::and32;
    using MacroAssemblerX86Common::or32;
    using MacroAssemblerX86Common::sub32;
    using MacroAssemblerX86Common::load32;
    using MacroAssemblerX86Common::store32;
    using MacroAssemblerX86Common::call;
    using MacroAssemblerX86Common::loadDouble;
    using MacroAssemblerX86Common::convertInt32ToDouble;

    void add32(Imm32 imm, AbsoluteAddress address)
    {
        move(ImmPtr(address.m_ptr), scratchRegister);
        add32(imm, Address(scratchRegister));
    }

    void and32(Imm32 imm, AbsoluteAddress address)
    {
        move(ImmPtr(address.m_ptr), scratchRegister);
        and32(imm, Address(scratchRegister));
    }

    void or32(Imm32 imm, AbsoluteAddress address)
    {
        move(ImmPtr(address.m_ptr), scratchRegister);
        or32(imm, Address(scratchRegister));
    }

    void sub32(Imm32 imm, AbsoluteAddress address)
    {
        move(ImmPtr(address.m_ptr), scratchRegister);
        sub32(imm, Address(scratchRegister));
    }

    void load32(void* address, RegisterID dest)
    {
        if (dest == X86Registers::eax)
            m_assembler.movl_mEAX(address);
        else {
            move(X86Registers::eax, dest);
            m_assembler.movl_mEAX(address);
            swap(X86Registers::eax, dest);
        }
    }

    void loadDouble(void* address, FPRegisterID dest)
    {
        move(ImmPtr(address), scratchRegister);
        loadDouble(scratchRegister, dest);
    }

    void convertInt32ToDouble(AbsoluteAddress src, FPRegisterID dest)
    {
        move(Imm32(*static_cast<int32_t*>(src.m_ptr)), scratchRegister);
        m_assembler.cvtsi2sd_rr(scratchRegister, dest);
    }

    void store32(Imm32 imm, void* address)
    {
        move(X86Registers::eax, scratchRegister);
        move(imm, X86Registers::eax);
        m_assembler.movl_EAXm(address);
        move(scratchRegister, X86Registers::eax);
    }

    Call call()
    {
        DataLabelPtr label = moveWithPatch(ImmPtr(0), scratchRegister);
        (void) label;

        Call result = Call(m_assembler.call(scratchRegister), Call::Linkable);
        ASSERT(differenceBetween(label, result) == REPTACH_OFFSET_CALL_R11);
        return result;
    }

    Call tailRecursiveCall()
    {
        DataLabelPtr label = moveWithPatch(ImmPtr(0), scratchRegister);
        (void) label;

        Jump newJump = Jump(m_assembler.jmp_r(scratchRegister));
        ASSERT(differenceBetween(label, newJump) == REPTACH_OFFSET_CALL_R11);
        return Call::fromTailJump(newJump);
    }

    Call makeTailRecursiveCall(Jump oldJump)
    {
        oldJump.link(this);

        DataLabelPtr label = moveWithPatch(ImmPtr(0), scratchRegister);
        (void) label;

        Jump newJump = Jump(m_assembler.jmp_r(scratchRegister));
        ASSERT(differenceBetween(label, newJump) == REPTACH_OFFSET_CALL_R11);
        return Call::fromTailJump(newJump);
    }

    void addPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.addq_rr(src, dest);
    }

    void addPtr(Imm32 imm, RegisterID srcDest)
    {
        m_assembler.addq_ir(imm.m_value, srcDest);
    }

    void addPtr(ImmPtr imm, RegisterID dest)
    {
        move(imm, scratchRegister);
        m_assembler.addq_rr(scratchRegister, dest);
    }

    void addPtr(Imm32 imm, RegisterID src, RegisterID dest)
    {
        m_assembler.leaq_mr(imm.m_value, src, dest);
    }

    void addPtr(Imm32 imm, Address address)
    {
        m_assembler.addq_im(imm.m_value, address.offset, address.base);
    }

    void addPtr(Imm32 imm, AbsoluteAddress address)
    {
        move(ImmPtr(address.m_ptr), scratchRegister);
        addPtr(imm, Address(scratchRegister));
    }

    void andPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.andq_rr(src, dest);
    }

    void andPtr(Imm32 imm, RegisterID srcDest)
    {
        m_assembler.andq_ir(imm.m_value, srcDest);
    }

    void orPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.orq_rr(src, dest);
    }

    void orPtr(ImmPtr imm, RegisterID dest)
    {
        move(imm, scratchRegister);
        m_assembler.orq_rr(scratchRegister, dest);
    }

    void orPtr(Imm32 imm, RegisterID dest)
    {
        m_assembler.orq_ir(imm.m_value, dest);
    }

    void subPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.subq_rr(src, dest);
    }

    void subPtr(Imm32 imm, RegisterID dest)
    {
        m_assembler.subq_ir(imm.m_value, dest);
    }

    void subPtr(ImmPtr imm, RegisterID dest)
    {
        move(imm, scratchRegister);
        m_assembler.subq_rr(scratchRegister, dest);
    }

    void xorPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.xorq_rr(src, dest);
    }

    void xorPtr(Imm32 imm, RegisterID srcDest)
    {
        m_assembler.xorq_ir(imm.m_value, srcDest);
    }


    void loadPtr(ImplicitAddress address, RegisterID dest)
    {
        m_assembler.movq_mr(address.offset, address.base, dest);
    }

    void loadPtr(BaseIndex address, RegisterID dest)
    {
        m_assembler.movq_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void loadPtr(void* address, RegisterID dest)
    {
        if (dest == X86Registers::eax)
            m_assembler.movq_mEAX(address);
        else {
            move(X86Registers::eax, dest);
            m_assembler.movq_mEAX(address);
            swap(X86Registers::eax, dest);
        }
    }

    DataLabel32 loadPtrWithAddressOffsetPatch(Address address, RegisterID dest)
    {
        m_assembler.movq_mr_disp32(address.offset, address.base, dest);
        return DataLabel32(this);
    }

    void storePtr(RegisterID src, ImplicitAddress address)
    {
        m_assembler.movq_rm(src, address.offset, address.base);
    }

    void storePtr(RegisterID src, BaseIndex address)
    {
        m_assembler.movq_rm(src, address.offset, address.base, address.index, address.scale);
    }

    void storePtr(RegisterID src, void* address)
    {
        if (src == X86Registers::eax)
            m_assembler.movq_EAXm(address);
        else {
            swap(X86Registers::eax, src);
            m_assembler.movq_EAXm(address);
            swap(X86Registers::eax, src);
        }
    }

    void storePtr(ImmPtr imm, ImplicitAddress address)
    {
        move(imm, scratchRegister);
        storePtr(scratchRegister, address);
    }

    DataLabel32 storePtrWithAddressOffsetPatch(RegisterID src, Address address)
    {
        m_assembler.movq_rm_disp32(src, address.offset, address.base);
        return DataLabel32(this);
    }

    void movePtrToDouble(RegisterID src, FPRegisterID dest)
    {
        m_assembler.movq_rr(src, dest);
    }

    void moveDoubleToPtr(FPRegisterID src, RegisterID dest)
    {
        m_assembler.movq_rr(src, dest);
    }

    void setPtr(Condition cond, RegisterID left, Imm32 right, RegisterID dest)
    {
        if (((cond == Equal) || (cond == NotEqual)) && !right.m_value)
            m_assembler.testq_rr(left, left);
        else
            m_assembler.cmpq_ir(right.m_value, left);
        m_assembler.setCC_r(x86Condition(cond), dest);
        m_assembler.movzbl_rr(dest, dest);
    }

    Jump branchPtr(Condition cond, RegisterID left, RegisterID right)
    {
        m_assembler.cmpq_rr(right, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchPtr(Condition cond, RegisterID left, ImmPtr right)
    {
        move(right, scratchRegister);
        return branchPtr(cond, left, scratchRegister);
    }

    Jump branchPtr(Condition cond, RegisterID left, Address right)
    {
        m_assembler.cmpq_mr(right.offset, right.base, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchPtr(Condition cond, AbsoluteAddress left, RegisterID right)
    {
        move(ImmPtr(left.m_ptr), scratchRegister);
        return branchPtr(cond, Address(scratchRegister), right);
    }

    Jump branchPtr(Condition cond, Address left, RegisterID right)
    {
        m_assembler.cmpq_rm(right, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchPtr(Condition cond, Address left, ImmPtr right)
    {
        move(right, scratchRegister);
        return branchPtr(cond, left, scratchRegister);
    }

    Jump branchTestPtr(Condition cond, RegisterID reg, RegisterID mask)
    {
        m_assembler.testq_rr(reg, mask);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTestPtr(Condition cond, RegisterID reg, Imm32 mask = Imm32(-1))
    {
        // if we are only interested in the low seven bits, this can be tested with a testb
        if (mask.m_value == -1)
            m_assembler.testq_rr(reg, reg);
        else if ((mask.m_value & ~0x7f) == 0)
            m_assembler.testb_i8r(mask.m_value, reg);
        else
            m_assembler.testq_i32r(mask.m_value, reg);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTestPtr(Condition cond, Address address, Imm32 mask = Imm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpq_im(0, address.offset, address.base);
        else
            m_assembler.testq_i32m(mask.m_value, address.offset, address.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTestPtr(Condition cond, BaseIndex address, Imm32 mask = Imm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpq_im(0, address.offset, address.base, address.index, address.scale);
        else
            m_assembler.testq_i32m(mask.m_value, address.offset, address.base, address.index, address.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }


    Jump branchAddPtr(Condition cond, RegisterID src, RegisterID dest)
    {
        ASSERT((cond == Overflow) || (cond == Zero) || (cond == NonZero));
        addPtr(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSubPtr(Condition cond, Imm32 imm, RegisterID dest)
    {
        ASSERT((cond == Overflow) || (cond == Zero) || (cond == NonZero));
        subPtr(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    DataLabelPtr moveWithPatch(ImmPtr initialValue, RegisterID dest)
    {
        m_assembler.movq_i64r(initialValue.asIntptr(), dest);
        return DataLabelPtr(this);
    }

    Jump branchPtrWithPatch(Condition cond, RegisterID left, DataLabelPtr& dataLabel, ImmPtr initialRightValue = ImmPtr(0))
    {
        dataLabel = moveWithPatch(initialRightValue, scratchRegister);
        return branchPtr(cond, left, scratchRegister);
    }

    Jump branchPtrWithPatch(Condition cond, Address left, DataLabelPtr& dataLabel, ImmPtr initialRightValue = ImmPtr(0))
    {
        dataLabel = moveWithPatch(initialRightValue, scratchRegister);
        return branchPtr(cond, left, scratchRegister);
    }

    DataLabelPtr storePtrWithPatch(ImmPtr initialValue, ImplicitAddress address)
    {
        DataLabelPtr label = moveWithPatch(initialValue, scratchRegister);
        storePtr(scratchRegister, address);
        return label;
    }

    Label loadPtrWithPatchToLEA(Address address, RegisterID dest)
    {
        Label label(this);
        loadPtr(address, dest);
        return label;
    }

    bool supportsFloatingPoint() const { return true; }
    // See comment on MacroAssemblerARMv7::supportsFloatingPointTruncate()
    bool supportsFloatingPointTruncate() const { return true; }

private:
    friend class LinkBuffer;
    friend class RepatchBuffer;

    static void linkCall(void* code, Call call, FunctionPtr function)
    {
        if (!call.isFlagSet(Call::Near))
            X86Assembler::linkPointer(code, X86Assembler::labelFor(call.m_jmp, -REPTACH_OFFSET_CALL_R11), function.value());
        else
            X86Assembler::linkCall(code, call.m_jmp, function.value());
    }

    static void repatchCall(CodeLocationCall call, CodeLocationLabel destination)
    {
        X86Assembler::repatchPointer(call.dataLabelPtrAtOffset(-REPTACH_OFFSET_CALL_R11).dataLocation(), destination.executableAddress());
    }

    static void repatchCall(CodeLocationCall call, FunctionPtr destination)
    {
        X86Assembler::repatchPointer(call.dataLabelPtrAtOffset(-REPTACH_OFFSET_CALL_R11).dataLocation(), destination.executableAddress());
    }

};

} // namespace JSC

#endif // ENABLE(ASSEMBLER)

#endif // MacroAssemblerX86_64_h
