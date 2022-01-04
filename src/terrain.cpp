#include "terrain.h"
#include <iostream>
#include "chunk.h"
#include <SDL2/SDL.h>
#include <random>
//
Terrain::Terrain()
    : m_chunks(), m_generatedTerrain()
{}

//Terrain::~Terrain() {
////    m_geomCube.destroyVBOdata();
//}
//
// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        return EMPTY;
//        throw std::out_of_range("Coordinates " + std::to_string(x) +
//                                " " + std::to_string(y) + " " +
//                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z) && x >= 0 && y >= 0 && z >= 0) {
        uPtr<Chunk> &c = getChunkAt(x, z);

        c->setBlockAt(x - c->chunkPos.x,
                      y,
                      z - c->chunkPos.z,
                      t);
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    int curr = SDL_GetTicks();
    uPtr<Chunk> chunk = mkU<Chunk>();
    Chunk *cPtr = chunk.get();
    cPtr->chunkPos = glm::vec4(x, 0, z, 1);
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    this->createTerrainZone(x, z);

    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
//    for(int x = minX; x < maxX; x += 16) {
//        for(int z = minZ; z < maxZ; z += 16) {
//            if(!hasChunkAt(x, z)) {
//                std::cout << "creating chunk X: " << x << " Z: " << z << std::endl;
//                this->instantiateChunkAt(x, z);
//            }
//
//            const uPtr<Chunk> &chunk = getChunkAt(x, z);
//            shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
//            chunk->setVBOdata();
//            shaderProgram->drawInterleaved(*chunk, false, 0);
//            chunk->setTVBOdata();
//            shaderProgram->drawInterleaved(*chunk, true, chunk->t_idx.size());
//        }
//    }

//    for(int x = minX; x < maxX; x += 16) {
//        for(int z = minZ; z < maxZ; z += 16) {
//            if(!hasChunkAt(x, z)) {
//                std::cout << "creating chunk X: " << x << " Z: " << z << std::endl;
//                this->instantiateChunkAt(x, z);
//            }
//
//            const uPtr<Chunk> &chunk = getChunkAt(x, z);
//            shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
//            chunk->setTVBOdata();
//            shaderProgram->drawInterleaved(*chunk, true, chunk->t_idx.size());
//        }
//    }
}

int Terrain::grasslandHeighField(int x, int z)
{
    glm::vec2 p = glm::vec2(x, z);

    float fy = (Noise::WorleyNoise(glm::vec2(x/480.f, z/480.f), std::vector<float>{0, -1, 1, 1}));
    fy = glm::min(143.f, (fy * (10) + 133));
    float fx = (Noise::WorleyNoise(glm::vec2(x/840.f, z/960.f), std::vector<float>{0, 1}));
    fx = glm::max(130.f, fx * 10 + 120);

    float fz = glm::abs(glm::simplex(p/128.f));
    fz = glm::max(143.f, fz * 6 + 143);

    float noise = (glm::simplex(p/160.f));
    float t = glm::smoothstep(0.05f, 0.45f, 0.5f * (1 + noise));

    noise = (glm::perlin(p/35.f));
    float t2 = glm::smoothstep(0.05f, 0.35f, 0.5f * (1 + noise));
    fy = glm::mix(fz, fy, t2);

    float height = glm::mix(fx, fy, t);

    return height;
}

int Terrain::mountainHeighField(int x, int z)
{
    glm::vec2 p = glm::vec2(x, z);

    // 180 ~ 225
    float fy = (0.5*(1 + Noise::perlinNoise(p/25.f))) * 25 + 190;
    float fx = Noise::fbm(glm::abs(glm::perlin(p/8.f))) * 10 + 200;

    return (fy > 200)?std::max(fx, fy): fy;
}

void Terrain::createBiomes(int x, int z) {

    glm::vec2 p = glm::vec2(x, z);
    float noise = (glm::simplex(p/1080.f));
    float t = glm::smoothstep(0.15f, 0.85f, 0.5f* (1 + noise));

    float h1 = this->mountainHeighField(x, z);
    float h2 = this->grasslandHeighField(x, z);

    float height = glm::mix(h1, h2, t);
    int y = height, i = 0;
    int baseLine = 80;

    for(; i <= baseLine; i++)
        setBlockAt(x, baseLine, z, BEDROCK);
    for(i = baseLine+1; i < y && i < 129; i++){
        glm::vec3 cavePos = glm::vec3(x / 100.f, i / 20.f, z / 150.f);
        bool isEmpty = (Noise::perlinNoise3D(cavePos) > 0);

        (isEmpty)?(i < baseLine+25)?setBlockAt(x, i, z, LAVA):setBlockAt(x, i, z, EMPTY):setBlockAt(x, i, z, STONE);
    }

    i = 129;
    if(t > 0.75){
        // Grassland
        for(; i < y; i++){
            setBlockAt(x, i, z, DIRT);
        }
        setBlockAt(x, i, z, GRASS);
    }
    else{
        // Mountain
        for(; i < y; i++){
            setBlockAt(x, i, z, STONE);
        }

        if(y > 200)
            setBlockAt(x, y, z, SNOW);
        else
            setBlockAt(x, y, z, STONE);
    }

    for(i = 138; i >= 128 && i >= y; i--){
        if(this->getBlockAt(x, i, z) == EMPTY && i <= 138)
            setBlockAt(x, i, z, WATER);
    }
}

void Terrain::createTerrainZone(int x, int z)
{
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//    std::cout << "terrain " << r << "  start " << SDL_GetTicks() << std::endl;
    x = 16 * static_cast<int>(glm::floor(x / 16.f));
    z = 16 * static_cast<int>(glm::floor(z / 16.f));

    for(int i = x; i < x + 16; i++){
        for(int j = z; j < z + 16; j++){
//            std::cout << "terrain " << r << "  start " << SDL_GetTicks() << std::endl;
            this->createBiomes(i, j);
//            std::cout << "terrain " << r << "  end " << SDL_GetTicks() << std::endl;
        }
    }

//    std::cout << "terrain " << r << "  end " << SDL_GetTicks() << std::endl;
}