#include "chunk.h"
#include "GLES2/gl2.h"
#include <iostream>
#include <SDL2/SDL.h>

Chunk::Chunk():
        m_blocks(),
        m_neighbors{
                {XPOS, nullptr},
                {XNEG, nullptr},
                {ZPOS, nullptr},
                {ZNEG, nullptr},
        },
        m_bufDataIdx(), m_bufData(), m_bufTransparentDataIdx(), m_bufTransparentData(),
        m_dataIdxGenerated(false), m_dataGenerated(false), m_transparentDataIdxGenerated(false),
        m_transparentDataGenerated(false), needUpdate(false)
        {
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

GLenum Chunk::drawMode()
{
//
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Chunk::dataCount() {
    return idx.size();
}

int Chunk::transparentDataCount() {
    return t_idx.size();
}

void Chunk::generateDataIdx() {
    m_dataIdxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    glGenBuffers(1, &m_bufDataIdx);
}

void Chunk::generateData() {
    m_dataGenerated = true;

    glGenBuffers(1, &m_bufData);
}

void Chunk::generateTransparentDataIdx() {
    m_transparentDataIdxGenerated = true;

    glGenBuffers(1, &m_bufTransparentDataIdx);
}

void Chunk::generateTransparentData() {
    m_transparentDataGenerated = true;

    glGenBuffers(1, &m_bufTransparentData);
}

bool Chunk::bindDataIdx() {
    if(m_dataIdxGenerated) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufDataIdx);
    }
    return m_dataIdxGenerated;
}
bool Chunk::bindData() {
    if (m_dataGenerated) {
        glBindBuffer(GL_ARRAY_BUFFER, m_bufData);
    }

    return m_dataGenerated;
}

bool Chunk::bindTransparentDataIdx() {
    if (m_transparentDataIdxGenerated) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransparentDataIdx);
    }

    return m_transparentDataIdxGenerated;
}

bool Chunk::bindTransparentData() {
    if (m_transparentDataGenerated) {
        glBindBuffer(GL_ARRAY_BUFFER, m_bufTransparentData);
    }

    return m_transparentDataGenerated;
}

void Chunk::createVBOdata() {
    for (auto& e : m_neighbors) {
        if (e.second == nullptr) {
            return;
        }
    }

    data.clear();
    idx.clear();

    t_data.clear();
    t_idx.clear();

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                if (getBlockAt(x, y, z) != EMPTY) {
                    uPtr<std::unordered_map<Direction, BlockType, EnumHash>> localNeighbors = getLocalNeighbors(x, y,
                                                                                                                z);
                    for (auto neighbors : *localNeighbors.get()) {
                        if (neighbors.second == EMPTY ||
                            (neighbors.second == WATER && getBlockAt(x, y, z) != WATER)) {
                            if (getBlockAt(x, y, z) == WATER) {
                                drawFace(neighbors.first,
                                         &t_data,
                                         &t_idx,
                                         glm::vec4(x, y, z, 0), getBlockAt(x, y, z));
                            } else {
                                drawFace(neighbors.first,
                                         &data,
                                         &idx,
                                         glm::vec4(x, y, z, 0), getBlockAt(x, y, z));
                            }
                        }
                    }
                }
            }
        }
    }

    vboSet = true;

    if (needUpdate) {
        needUpdate = false;
        if (m_dataGenerated) glDeleteBuffers(1, &m_bufData);
        if (m_dataIdxGenerated) glDeleteBuffers(1, &m_bufDataIdx);
        if (m_transparentDataGenerated) glDeleteBuffers(1, &m_bufTransparentData);
        if (m_transparentDataIdxGenerated) glDeleteBuffers(1, &m_bufTransparentDataIdx);
        m_dataGenerated = false;
        m_dataIdxGenerated = false;
        m_transparentDataGenerated = false;
        m_transparentDataIdxGenerated = false;
    }
}

void Chunk::setVBOdata() {
    if (!m_dataGenerated || !m_dataIdxGenerated) {
        generateDataIdx();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufDataIdx);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

        generateData();
        glBindBuffer(GL_ARRAY_BUFFER, m_bufData);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), data.data(), GL_STATIC_DRAW);
    }
}

void Chunk::setTVBOdata() {
    if (!m_transparentDataIdxGenerated || !m_transparentDataGenerated) {
        generateTransparentDataIdx();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransparentDataIdx);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, t_idx.size() * sizeof(GLuint), t_idx.data(), GL_STATIC_DRAW);

        generateTransparentData();
        glBindBuffer(GL_ARRAY_BUFFER, m_bufTransparentData);
        glBufferData(GL_ARRAY_BUFFER, t_data.size() * sizeof(glm::vec4), t_data.data(), GL_STATIC_DRAW);
    }
}

void Chunk::drawFace(
        Direction direction,
        std::vector<glm::vec4>* data,
        std::vector<GLuint>* idx,
        glm::vec4 blockPos,
        BlockType blockType) {

    int originalPosSize = data->size() / 3;

    switch(direction) {
        case(XPOS):
            data->push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
        case(XNEG):
            data->push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(-1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(-1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(-1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(-1, 0, 0, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
        case(YPOS):
            data->push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 1, 0, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 1, 0, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 1, 0, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 1, 0, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
        case(YNEG):
            data->push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, -1, 0, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, -1, 0, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, -1, 0, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, -1, 0, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
        case(ZPOS):
            data->push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, 1, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, 1, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, 1, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, 1, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
        case(ZNEG):
            data->push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, -1, 0));
            data->push_back(getUVCoord(blockType, BR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, -1, 0));
            data->push_back(getUVCoord(blockType, BL, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, -1, 0));
            data->push_back(getUVCoord(blockType, TR, direction == YPOS, direction == YNEG));

            data->push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
            data->push_back(glm::vec4(0, 0, -1, 0));
            data->push_back(getUVCoord(blockType, TL, direction == YPOS, direction == YNEG));

            break;
    }

    // fill idx buffer to make square
    idx->push_back(0 + originalPosSize);
    idx->push_back(1 + originalPosSize);
    idx->push_back(2 + originalPosSize);
    idx->push_back(0 + originalPosSize);
    idx->push_back(2 + originalPosSize);
    idx->push_back(3 + originalPosSize);
}

glm::vec4 Chunk::getUVCoord(BlockType blockType, Corner corner, bool isTop, bool isBottom) {
    glm::vec4 blockCoord;
    switch(corner) {
        case BR:
            blockCoord = glm::vec4(1.f, 0.f, 0.f, 0.f);
            break;
        case BL:
            blockCoord = glm::vec4(0.f, 0.f, 0.f, 0.f);
            break;
        case TR:
            blockCoord = glm::vec4(0.f, 1.f, 0.f, 0.f);
            break;
        case TL:
            blockCoord = glm::vec4(1.f, 1.f, 0.f, 0.f);
            break;
    }

    switch(blockType) {
        case GRASS:
            if (isTop) {
                blockCoord += glm::vec4(8.f, 13.f, 0.f, 0.f);
            } else if (isBottom) {
                blockCoord += glm::vec4(2.f, 15.f, 0.f, 0.f);
            } else {
                blockCoord += glm::vec4(3.f, 15.f, 0.f, 0.f);
            }
            break;
        case DIRT:
            blockCoord += glm::vec4(2.f, 15.f, 0.f, 0.f);
            break;
        case STONE:
            blockCoord += glm::vec4(1.f, 15.f, 0.f, 0.f);
            break;
        case WATER:
            blockCoord += glm::vec4(13.f, 3.f, 16.f, 13.f);
            break;
        case SNOW:
            blockCoord += glm::vec4(2.f, 11.f, 0.f, 0.f);
            break;
        case LAVA:
            blockCoord += glm::vec4(13.f, 1.f, 16.f, 13.f);
            break;
        case BEDROCK:
            blockCoord += glm::vec4(1.f, 14.f, 0.f, 0.f);
            break;
        case SAND:
            blockCoord += glm::vec4(2.f, 14.f, 0.f, 0.f);
            break;
        default:
            // Other block types are not yet handled, so we default to debug purple
            blockCoord += glm::vec4(7.f, 1.f, 0.f, 0.f);
            break;
    }

    blockCoord /= 16.f;
    return blockCoord;
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if (x >= 0 && x < 16 && y >= 0 && y < 256 && z >= 0 && z < 16)
        return m_blocks.at(x + 16 * y + 16 * 256 * z);
    else {
        if (x < 0 && m_neighbors.at(XNEG) != nullptr) {
            return m_neighbors.at(XNEG)->getBlockAt(static_cast<unsigned int>(15), y, z);
        } else if (x >= 16 && m_neighbors.at(XPOS) != nullptr) {
            return m_neighbors.at(XPOS)->getBlockAt(static_cast<unsigned int>(0), y, z);
        } else if (z < 0 && m_neighbors.at(ZNEG) != nullptr) {
            return m_neighbors.at(ZNEG)->getBlockAt(x, y, static_cast<unsigned int>(15));
        } else if (z >= 16 && m_neighbors.at(ZPOS) != nullptr) {
            return m_neighbors.at(ZPOS)->getBlockAt(x, y, static_cast<unsigned int>(0));
        } else {
            // if we don't have a neighbor then we can assume it's full as we don't care if
            // the edges of the world are filled in or not
            // return GRASS;
            return EMPTY;
        }
    }
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
        {XPOS, XNEG},
        {XNEG, XPOS},
        {YPOS, YNEG},
        {YNEG, YPOS},
        {ZPOS, ZNEG},
        {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

uPtr<std::unordered_map<Direction, BlockType, EnumHash>> Chunk::getLocalNeighbors(int x, int y, int z) {
    std::unordered_map<Direction, BlockType, EnumHash> localNeighbors = {};
    localNeighbors[XPOS] = getBlockAt(x + 1, y, z);
    localNeighbors[XNEG] = getBlockAt(x - 1, y, z);
    localNeighbors[YPOS] = getBlockAt(x, y + 1, z);
    localNeighbors[YNEG] = getBlockAt(x, y - 1, z);
    localNeighbors[ZPOS] = getBlockAt(x, y, z + 1);
    localNeighbors[ZNEG] = getBlockAt(x, y, z - 1);
    return mkU<std::unordered_map<Direction, BlockType, EnumHash>>(localNeighbors);
}

//#include "chunk.h"
//#include "GLES2/gl2.h"
//#include <iostream>
//
//Chunk::Chunk() :
//        Drawable(),
//        m_blocks(),
//        m_neighbors{
//                {XPOS, nullptr},
//                {XNEG, nullptr},
//                {ZPOS, nullptr},
//                {ZNEG, nullptr},
//        }, vboSet(false) {
//    std::fill_n(m_blocks.begin(), 65536, EMPTY);
//}
//
//void Chunk::createVBOdata() {
//    if (vboSet) {
//        std::cout << "ERROR REGENERATING VBO DATA" << std::endl;
//        return;
//    }
//
//    data.clear();
//    idx.clear();
//    int blockCount = 0;
//
//    for (int x = 0; x < 16; x++) {
//        for (int y = 0; y < 256; y++) {
//            for (int z = 0; z < 16; z++) {
//                if (getBlockAt(x, y, z) != EMPTY) {
//                    uPtr<std::unordered_map<Direction, BlockType, EnumHash>> localNeighbors = getLocalNeighbors(x, y,
//                                                                                                                z);
//                    for (auto neighbors : *localNeighbors.get()) {
//                        if (neighbors.second == EMPTY) {
//                            drawFace(neighbors.first,
//                                     &data,
//                                     &idx,
//                                     blockCount,
//                                     glm::vec4(x, y, z, 0), getBlockAt(x, y, z));
//                            blockCount += 4;
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    vboSet = true;
//    std::cout << "createVBOData" << data.size() << std::endl;
//}
//
//void Chunk::setVBOdata() {
//    if (m_bufIdx == -1 || m_bufPos == -1) {
//        m_count = idx.size();
//        std::cout << "DRAWING " << m_count << std::endl;
//
//        // Create a VBO on our GPU and store its handle in bufIdx
//        generateIdx();
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
//
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
//
//        generatePos();
//        glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
//        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), data.data(), GL_STATIC_DRAW);
//    }
//}
//
//void Chunk::drawFace(
//        Direction direction,
//        std::vector<glm::vec4> *data,
//        std::vector<GLuint> *idx,
//        int blockCount,
//        glm::vec4 blockPos,
//        BlockType blockType) {
//
//    glm::vec4 blockCol = getColor(blockType);
//    std::vector<glm::vec4> posVectors;
//    glm::vec4 norVector;
//
//    switch (direction) {
//        case (XPOS):
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(1, 0, 0, 0);
//            break;
//        case (XNEG):
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(-1, 0, 0, 0);
//            break;
//        case (YPOS):
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(0, 1, 0, 0);
//            break;
//        case (YNEG):
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(0, -1, 0, 0);
//            break;
//        case (ZPOS):
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(0, 0, 1, 0);
//            break;
//        case (ZNEG):
//            posVectors.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//            posVectors.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) + blockPos);
//
//            norVector = glm::vec4(0, 0, -1, 0);
//            break;
//    }
//
//    for (int i = 0; i < 12; i++) {
//        if (i % 3 == 0) {
//            data->push_back(posVectors[i/3]);
//        } else if (i % 3 == 1) {
//            data->push_back(norVector);
//        } else {
//            data->push_back(blockCol);
//        }
//    }
//
//    // fill idx buffer to make square
//    idx->push_back(0 + blockCount);
//    idx->push_back(1 + blockCount);
//    idx->push_back(2 + blockCount);
//    idx->push_back(0 + blockCount);
//    idx->push_back(2 + blockCount);
//    idx->push_back(3 + blockCount);
//}
//
//glm::vec4 Chunk::getColor(BlockType blockType) {
//    switch (blockType) {
//        case GRASS:
//            return glm::vec4(95.f, 159.f, 53.f, 0.f) / 255.f;
//        case DIRT:
//            return glm::vec4(121.f, 85.f, 58.f, 0.f) / 255.f;
//        case STONE:
//            return glm::vec4(0.5f, 0.5f, 0.5f, 0.f);
//        case WATER:
//            return glm::vec4(0.f, 0.f, 0.75f, 0.f);
//        case SNOW:
//            return glm::vec4(0.9f, 0.9f, 0.9f, 0.f);
//        default:
//            // Other block types are not yet handled, so we default to debug purple
//            return glm::vec4(1.f, 0.f, 1.f, 0.f);
//    }
//}
//
//// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
//BlockType Chunk::getBlockAt(int x, int y, int z) const {
//    if (x >= 0 && x < 16 && y >= 0 && y < 256 && z >= 0 && z < 16)
//        return m_blocks.at(x + 16 * y + 16 * 256 * z);
//    else {
//        if (x < 0 && m_neighbors.at(XNEG) != nullptr) {
//            return m_neighbors.at(XNEG)->getBlockAt(static_cast<unsigned int>(15), y, z);
//        } else if (x >= 16 && m_neighbors.at(XPOS) != nullptr) {
//            return m_neighbors.at(XPOS)->getBlockAt(static_cast<unsigned int>(0), y, z);
//        } else if (z < 0 && m_neighbors.at(ZNEG) != nullptr) {
//            return m_neighbors.at(ZNEG)->getBlockAt(x, y, static_cast<unsigned int>(15));
//        } else if (z >= 16 && m_neighbors.at(ZPOS) != nullptr) {
//            return m_neighbors.at(ZPOS)->getBlockAt(x, y, static_cast<unsigned int>(0));
//        } else {
//            return EMPTY;
//        }
//    }
//}
//
//// Does bounds checking with at()
//void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
//    if (x + 16 * y + 16 * 256 * z < 65536) {
//        m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
//    }
//}
//
//
//const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection{
//        {XPOS, XNEG},
//        {XNEG, XPOS},
//        {YPOS, YNEG},
//        {YNEG, YPOS},
//        {ZPOS, ZNEG},
//        {ZNEG, ZPOS}
//};
//
//void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
//    if (neighbor != nullptr) {
//        this->m_neighbors[dir] = neighbor.get();
//        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
//    }
//}
//
//uPtr<std::unordered_map<Direction, BlockType, EnumHash>> Chunk::getLocalNeighbors(int x, int y, int z) {
//    std::unordered_map<Direction, BlockType, EnumHash> localNeighbors = {};
//    localNeighbors[XPOS] = getBlockAt(x + 1, y, z);
//    localNeighbors[XNEG] = getBlockAt(x - 1, y, z);
//    localNeighbors[YPOS] = getBlockAt(x, y + 1, z);
//    localNeighbors[YNEG] = getBlockAt(x, y - 1, z);
//    localNeighbors[ZPOS] = getBlockAt(x, y, z + 1);
//    localNeighbors[ZNEG] = getBlockAt(x, y, z - 1);
//    return mkU<std::unordered_map<Direction, BlockType, EnumHash>>(localNeighbors);
//}
