#include "PDS.h"

s32 FP_GetIntegerPortion(fixedPoint& FP)
{
    return FP.asS32() >> 16;
}

fixedPoint FP_Div(s32 divident, fixedPoint divisor)
{
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = divisor;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    u32 result = DVDNTUL;
    VCRDIV = 0;
    */

    if (divisor == 0)
    {
        if (divident >= 0)
        {
            return 0x7FFFFFFF;
        }
        else
        {
            return 0x80000000;
        }
    }

    return fixedPoint::fromS32((((s64)divident) * 0x10000) / divisor.asS32());
}

sVec3_FP FP_Div(sVec3_FP divident, fixedPoint divisor)
{
    sVec3_FP result;
    result[0] = FP_Div(divident[0], divisor);
    result[1] = FP_Div(divident[1], divisor);
    result[2] = FP_Div(divident[2], divisor);

    return result;
}

fixedPoint dot3_FP(const sVec3_FP* r4, const sVec3_FP* r5)
{
    s64 acc = 0;
    acc += (*r4)[0] * (s64)(*r5)[0];
    acc += (*r4)[1] * (s64)(*r5)[1];
    acc += (*r4)[2] * (s64)(*r5)[2];

    return (s32)(acc >> 16);
}

fixedPoint MTH_Product3d_FP(const sVec3_FP& r4, const sVec3_FP& r5)
{
    return dot3_FP(&r4, &r5);
}

s64 gDivident;
s64 gDivisor;

void asyncDivStart(s32 r4, fixedPoint r5)
{
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = r5;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    */

    gDivident = (((s64)r4) << 16);
    gDivisor = r5;
}

void asyncDivStart_integer(s32 r4, s32 r5)
{
    gDivident = r4;
    gDivisor = r5;
}

fixedPoint asyncDivEnd()
{
    if (gDivisor == 0)
    {
        if (gDivident >= 0)
        {
            return 0x7FFFFFFF;
        }
        else
        {
            return 0x80000000;
        }
    }
    return fixedPoint::fromS32(gDivident / gDivisor);
}

fixedPoint performDivision(fixedPoint r0, fixedPoint r1)
{
    return r1 / r0;
}

s64 MUL_FP(const fixedPoint& A, const fixedPoint& B)
{
    return (s64)A.asS32() * (s64)B.asS32();
}

fixedPoint MTH_Mul_5_6(fixedPoint a, fixedPoint b, fixedPoint c)
{
    return MTH_Mul(a, MTH_Mul(b, c));
}

fixedPoint MTH_Mul(fixedPoint a, fixedPoint b)
{
    return fixedPoint(((s64)a.asS32() * (s64)b.asS32()) >> 16);
}

sVec3_FP MTH_Mul(const fixedPoint& a, const sVec3_FP& b)
{
    sVec3_FP temp;
    temp[0] = MTH_Mul(a, b[0]);
    temp[1] = MTH_Mul(a, b[1]);
    temp[2] = MTH_Mul(a, b[2]);
    return temp;
}

sVec3_FP MTH_Mul(const sVec3_FP& a, const sVec3_FP& b)
{
    sVec3_FP temp;
    temp[0] = MTH_Mul(a[0], b[0]);
    temp[1] = MTH_Mul(a[1], b[1]);
    temp[2] = MTH_Mul(a[2], b[2]);
    return temp;
}

sVec3_FP MTH_Mul(const sVec3_FP& a, const fixedPoint& b)
{
    sVec3_FP temp;
    temp[0] = MTH_Mul(a[0], b);
    temp[1] = MTH_Mul(a[1], b);
    temp[2] = MTH_Mul(a[2], b);
    return temp;
}

fixedPoint FP_Pow2(fixedPoint r4)
{
    s64 result = ((s64)r4.asS32()) * ((s64)r4.asS32());
    return fixedPoint::fromS32(result >> 16);
}



void Imgui_FP(const char* label, fixedPoint* pFP)
{
    float fValue = pFP->toFloat();
    if (ImGui::InputFloat(label, &fValue, 0.01, 0.1))
    {
        pFP->m_value = fValue * 0x10000;
    }
}

void Imgui_FP_Angle(const char* label, fixedPoint* pFP)
{
    // 1 = sin(1024)
    // 1024 = pi/2
    // conversion is (pi/2)/1024
    float fValueInRad = pFP->toFloat() * ((glm::pi<float>() / 2.f) / 1024.f);
    float fValueInDegree = glm::degrees<float>(fValueInRad);
    if (ImGui::InputFloat(label, &fValueInDegree, 0.01, 0.1))
    {
        fValueInRad = glm::radians<float>(fValueInDegree);

        pFP->m_value = (fValueInRad / ((glm::pi<float>() / 2.f) / 1024.f)) * 0x10000;
    }
}

void Imgui_Vec3FP(sVec3_FP* pVector)
{
    ImGui::PushItemWidth(100);
    Imgui_FP("x", &pVector->m0_X); ImGui::SameLine();
    Imgui_FP("y", &pVector->m4_Y); ImGui::SameLine();
    Imgui_FP("z", &pVector->m8_Z);
    ImGui::PopItemWidth();
}

void Imgui_Vec3FP(const char* name, sVec3_FP* pVector)
{
    ImGui::Text(name); ImGui::SameLine();
    ImGui::PushID(name);
    Imgui_Vec3FP(pVector);
    ImGui::PopID();
}
