#pragma once

//typedef s32 fixedPoint;

class fp16
{
public:
    s16 m_value;
    
    fp16() : m_value(0)
    {
        
    }
    
    fp16(s16 newValue) : m_value(newValue)
    {
        
    }
    
    fp16 operator - () const
    {
        fp16 newFixedPoint;
        newFixedPoint.m_value = -m_value;
        return newFixedPoint;
    }

    fp16 operator += (const fp16& otherValue)
    {
        m_value += otherValue.m_value;
        return *this;
    }

    fp16 operator -= (const fp16& otherValue)
    {
        m_value -= otherValue.m_value;
        return *this;
    }

    fp16 operator *= (const fp16& otherValue)
    {
        m_value *= otherValue.m_value;
        return *this;
    }
    
    s32 toFP32() const
    {
        return ((s32)m_value) * 0x10000;
    }
    
    operator s16() const
    {
        return m_value;
    }
    
    operator s16()
    {
        return m_value;
    }
};

class fixedPoint
{
private:
    //u16 m_integer;
    //u16 m_fractional;

public:
    s32 m_value;

    fixedPoint() : m_value(0)
    {

    }

    fixedPoint(s32 newValue) : m_value(newValue)
    {

    }

    fixedPoint operator - () const
    {
        fixedPoint newFixedPoint;
        newFixedPoint.m_value = -m_value;
        return newFixedPoint;
    }

    fixedPoint operator += (const fixedPoint& otherValue)
    {
        m_value += otherValue.m_value;
        return *this;
    }

    fixedPoint operator -= (const fixedPoint& otherValue)
    {
        m_value -= otherValue.m_value;
        return *this;
    }

    fixedPoint operator *= (const fixedPoint& otherValue)
    {
        m_value *= otherValue.m_value;
        return *this;
    }

    fixedPoint operator /= (const fixedPoint& otherValue)
    {
        m_value /= otherValue.m_value;
        return *this;
    }

    s32 asS32() const
    {
        return m_value;
    }

    s32 getInteger() const
    {
        return m_value >> 16;
    }

    static fixedPoint fromS32(s32 input)
    {
        fixedPoint value;
        value.m_value = input;
        return value;
    }

    fixedPoint normalized() const
    {
        if (m_value & 0x8000000)
        {
            return m_value | 0xF0000000;
        }
        else
        {
            return m_value & 0x0FFFFFFF;
        }
    }

    fixedPoint getAbs()
    {
        if (m_value >= 0)
            return m_value;
        else
            return -m_value;
    }

    operator s32() const
    {
        return m_value;
    }

    float toFloat() const
    {
        return (float)m_value / (float)(1 << 16);
    }
    
    void setFromInteger(s16 inputInteger)
    {
        m_value = ((s32)inputInteger) * 0x10000;
    }
};

fixedPoint MTH_Mul_5_6(fixedPoint a, fixedPoint b, fixedPoint c);
fixedPoint MTH_Mul(fixedPoint a, fixedPoint b);
s64 MUL_FP(const fixedPoint& A, const fixedPoint& B);

s32 FP_GetIntegerPortion(fixedPoint& FP);
fixedPoint FP_Div(s32 divident, fixedPoint divisor);
fixedPoint performDivision(fixedPoint r0, fixedPoint r1);
s32 atan2_FP(s32 y, s32 x);
s32 atan2(s32 y, s32 x);
fixedPoint FP_Pow2(fixedPoint r4);

void asyncDivStart(s32 r4, fixedPoint r5);
void asyncDivStart_integer(s32 r4, s32 r5);
fixedPoint asyncDivEnd();

int convertCutsceneRotationComponent(int param_1, int param_2);

