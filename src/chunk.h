#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include "Drawable.h"

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, BEDROCK, LAVA, SAND, UNDEFINED
};

enum Corner : unsigned char
{
    BL, BR, TL, TR
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    std::vector<glm::vec4> data {};
    std::vector<glm::vec4> t_data {};
    std::vector<GLuint> idx {};

public:
    std::vector<GLuint> t_idx {};
    Chunk();

    GLuint m_bufDataIdx;
    GLuint m_bufData;
    GLuint m_bufTransparentDataIdx;
    GLuint m_bufTransparentData;

    bool m_dataIdxGenerated;
    bool m_dataGenerated;
    bool m_transparentDataIdxGenerated;
    bool m_transparentDataGenerated;

    void generateDataIdx();
    void generateData();
    void generateTransparentDataIdx();
    void generateTransparentData();

    bool bindDataIdx();
    bool bindData();
    bool bindTransparentDataIdx();
    bool bindTransparentData();

    GLenum drawMode();

    int dataCount();
    int transparentDataCount();

    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    // used to store the location of the chunk
    glm::vec4 chunkPos;

    bool vboSet;
    bool needUpdate;
    void createVBOdata();
    void setTVBOdata();
    void setVBOdata();

    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    // gets all the blocks adjacent to the input block
    uPtr<std::unordered_map<Direction, BlockType, EnumHash>> getLocalNeighbors(int x, int y, int z);

    // draws a face
    void drawFace(
            Direction direction,
            std::vector<glm::vec4>* data,
            std::vector<GLuint>* idx,
            glm::vec4 blockPos,
            BlockType blockType);

    // gets a color for a block type will be removed when textures are added
    glm::vec4 getUVCoord(BlockType blockType, Corner corner, bool isTop = 1, bool isBottom = 0);
};
