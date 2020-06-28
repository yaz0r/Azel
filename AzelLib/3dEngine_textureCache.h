struct s_vertice
{
    float m_coordianges[3];
    float m_originalVertices[4];
    float m_color[4];
}; 

struct s_quad
{
    u8* model;
    char isInitialized;
    s_vertice m_vertices[4];
    u16 CMDCTRL;
    u16 CMDPMOD;
    u16 CMDCOLR;
    u16 CMDSRCA;
    u16 CMDSIZE;

    u32 gouraudPointer;

    // world space
    float m_modelMatrix[4 * 4];
};

u32* decodeVdp1Quad(s_quad quad, u16& textureWidth, u16& textureHeight);
unsigned int getTextureForQuad(s_quad& quad);
