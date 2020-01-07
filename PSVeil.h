#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer buf
// {
//
//   float Threshold;                   // Offset:    0 Size:     4
//   float Opacity;                     // Offset:    4 Size:     4
//   float unused1;                     // Offset:    8 Size:     4 [unused]
//   float unused2;                     // Offset:   12 Size:     4 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// samLinear                         sampler      NA          NA             s0      1 
// tx                                texture  float4          2d             t0      1 
// previousTx                        texture  float4          2d             t1      1 
// buf                               cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float       
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   xyzw        0   TARGET   float   xyzw
//
//
// Constant buffer to DX9 shader constant mappings:
//
// Target Reg Buffer  Start Reg # of Regs        Data Conversion
// ---------- ------- --------- --------- ----------------------
// c0         cb0             0         1  ( FLT, FLT, FLT, FLT)
//
//
// Sampler/Resource to DX9 shader sampler mappings:
//
// Target Sampler Source Sampler  Source Resource
// -------------- --------------- ----------------
// s0             s0              t0               
// s1             s0              t1               
//
//
// Level9 shader bytecode:
//
    ps_2_0
    def c1, -1, 1, 0.333333343, 0
    dcl t0.xy
    dcl_2d s0
    dcl_2d s1
    texld r0, t0, s1
    texld r1, t0, s0
    add r1.w, r0.w, c1.x
    add r0.x, -r0.w, c1.y
    rcp r0.x, r0.x
    mul r1.w, r1.w, r1.w
    mul r2.xyz, r0.x, r1
    cmp r0.xyz, -r1.w, c1.y, r2
    add r0.x, r0.y, r0.x
    add r0.x, r0.z, r0.x
    mul r0.y, r0.x, c1.z
    mov r0.z, c1.z
    mad r0.x, r0.x, -r0.z, c0.x
    rcp r0.y, r0.y
    mul r0.z, c0.y, c0.x
    mad r0.y, r0.z, -r0.y, c0.y
    cmp r0.w, r0.x, c1.w, r0.y
    mov r0.xyz, c1.w
    mov oC0, r0

// approximately 19 instruction slots used (2 texture, 17 arithmetic)
ps_4_0
dcl_constantbuffer CB0[1], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_input_ps linear v1.xy
dcl_output o0.xyzw
dcl_temps 2
sample r0.xyzw, v1.xyxx, t0.xyzw, s0
sample r1.xyzw, v1.xyxx, t1.xyzw, s0
add r0.w, -r1.w, l(1.000000)
eq r1.x, r1.w, l(1.000000)
div r0.xyz, r0.xyzx, r0.wwww
movc r0.xyz, r1.xxxx, l(1.000000,1.000000,1.000000,0), r0.xyzx
add r0.x, r0.y, r0.x
add r0.x, r0.z, r0.x
mul r0.x, r0.x, l(0.333333)
mul r0.y, cb0[0].y, cb0[0].x
div r0.y, r0.y, r0.x
lt r0.x, cb0[0].x, r0.x
add r0.y, -r0.y, cb0[0].y
and o0.w, r0.y, r0.x
mov o0.xyz, l(0,0,0,0)
ret 
// Approximately 16 instruction slots used
#endif

const BYTE g_PSV[] =
{
     68,  88,  66,  67,  24, 146, 
    237, 247,  78, 105, 225,  39, 
    192, 194,  48,  76,  53,  57, 
    228, 194,   1,   0,   0,   0, 
    216,   6,   0,   0,   6,   0, 
      0,   0,  56,   0,   0,   0, 
    232,   1,   0,   0,  60,   4, 
      0,   0, 184,   4,   0,   0, 
     76,   6,   0,   0, 164,   6, 
      0,   0,  65, 111, 110,  57, 
    168,   1,   0,   0, 168,   1, 
      0,   0,   0,   2, 255, 255, 
    112,   1,   0,   0,  56,   0, 
      0,   0,   1,   0,  44,   0, 
      0,   0,  56,   0,   0,   0, 
     56,   0,   2,   0,  36,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   2, 255, 255,  81,   0, 
      0,   5,   1,   0,  15, 160, 
      0,   0, 128, 191,   0,   0, 
    128,  63, 171, 170, 170,  62, 
      0,   0,   0,   0,  31,   0, 
      0,   2,   0,   0,   0, 128, 
      0,   0,   3, 176,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      0,   8,  15, 160,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      1,   8,  15, 160,  66,   0, 
      0,   3,   0,   0,  15, 128, 
      0,   0, 228, 176,   1,   8, 
    228, 160,  66,   0,   0,   3, 
      1,   0,  15, 128,   0,   0, 
    228, 176,   0,   8, 228, 160, 
      2,   0,   0,   3,   1,   0, 
      8, 128,   0,   0, 255, 128, 
      1,   0,   0, 160,   2,   0, 
      0,   3,   0,   0,   1, 128, 
      0,   0, 255, 129,   1,   0, 
     85, 160,   6,   0,   0,   2, 
      0,   0,   1, 128,   0,   0, 
      0, 128,   5,   0,   0,   3, 
      1,   0,   8, 128,   1,   0, 
    255, 128,   1,   0, 255, 128, 
      5,   0,   0,   3,   2,   0, 
      7, 128,   0,   0,   0, 128, 
      1,   0, 228, 128,  88,   0, 
      0,   4,   0,   0,   7, 128, 
      1,   0, 255, 129,   1,   0, 
     85, 160,   2,   0, 228, 128, 
      2,   0,   0,   3,   0,   0, 
      1, 128,   0,   0,  85, 128, 
      0,   0,   0, 128,   2,   0, 
      0,   3,   0,   0,   1, 128, 
      0,   0, 170, 128,   0,   0, 
      0, 128,   5,   0,   0,   3, 
      0,   0,   2, 128,   0,   0, 
      0, 128,   1,   0, 170, 160, 
      1,   0,   0,   2,   0,   0, 
      4, 128,   1,   0, 170, 160, 
      4,   0,   0,   4,   0,   0, 
      1, 128,   0,   0,   0, 128, 
      0,   0, 170, 129,   0,   0, 
      0, 160,   6,   0,   0,   2, 
      0,   0,   2, 128,   0,   0, 
     85, 128,   5,   0,   0,   3, 
      0,   0,   4, 128,   0,   0, 
     85, 160,   0,   0,   0, 160, 
      4,   0,   0,   4,   0,   0, 
      2, 128,   0,   0, 170, 128, 
      0,   0,  85, 129,   0,   0, 
     85, 160,  88,   0,   0,   4, 
      0,   0,   8, 128,   0,   0, 
      0, 128,   1,   0, 255, 160, 
      0,   0,  85, 128,   1,   0, 
      0,   2,   0,   0,   7, 128, 
      1,   0, 255, 160,   1,   0, 
      0,   2,   0,   8,  15, 128, 
      0,   0, 228, 128, 255, 255, 
      0,   0,  83,  72,  68,  82, 
     76,   2,   0,   0,  64,   0, 
      0,   0, 147,   0,   0,   0, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  90,   0, 
      0,   3,   0,  96,  16,   0, 
      0,   0,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   1,   0, 
      0,   0,  85,  85,   0,   0, 
     98,  16,   0,   3,  50,  16, 
     16,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,  69,   0,   0,   9, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  69,   0,   0,   9, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  16,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     16,   0,   1,   0,   0,   0, 
      0,  96,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   8, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16, 128, 
     65,   0,   0,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  24,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  14,   0,   0,   7, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0, 246,  15, 
     16,   0,   0,   0,   0,   0, 
     55,   0,   0,  12, 114,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0, 171, 170, 170,  62, 
     56,   0,   0,   9,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     14,   0,   0,   7,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  49,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   9,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16, 128,  65,   0, 
      0,   0,   0,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   7, 130,  32, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 114,  32,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  83,  84, 
     65,  84, 116,   0,   0,   0, 
     16,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  10,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  82,  68,  69,  70, 
    140,   1,   0,   0,   1,   0, 
      0,   0, 184,   0,   0,   0, 
      4,   0,   0,   0,  28,   0, 
      0,   0,   0,   4, 255, 255, 
      0,   1,   0,   0, 100,   1, 
      0,   0, 156,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 166,   0, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  13,   0,   0,   0, 
    169,   0,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   1,   0,   0,   0, 
      1,   0,   0,   0,  13,   0, 
      0,   0, 180,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 115,  97, 
    109,  76, 105, 110, 101,  97, 
    114,   0, 116, 120,   0, 112, 
    114, 101, 118, 105, 111, 117, 
    115,  84, 120,   0,  98, 117, 
    102,   0, 180,   0,   0,   0, 
      4,   0,   0,   0, 208,   0, 
      0,   0,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  48,   1,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     60,   1,   0,   0,   0,   0, 
      0,   0,  76,   1,   0,   0, 
      4,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     60,   1,   0,   0,   0,   0, 
      0,   0,  84,   1,   0,   0, 
      8,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     60,   1,   0,   0,   0,   0, 
      0,   0,  92,   1,   0,   0, 
     12,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     60,   1,   0,   0,   0,   0, 
      0,   0,  84, 104, 114, 101, 
    115, 104, 111, 108, 100,   0, 
    171, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     79, 112,  97,  99, 105, 116, 
    121,   0, 117, 110, 117, 115, 
    101, 100,  49,   0, 117, 110, 
    117, 115, 101, 100,  50,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  49, 
     48,  46,  49,   0,  73,  83, 
     71,  78,  80,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  68,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   3,   3, 
      0,   0,  83,  86,  95,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0, 171, 
    171, 171,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  97, 114, 
    103, 101, 116,   0, 171, 171
};
