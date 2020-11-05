#include <cstdio>
#include <string>
#include <stdexcept>
#include <dirent.h>

#define i16 short int
#define u8 unsigned char
#define u32 unsigned int
#define u16 unsigned short int

template<typename T>
struct Vec4 {
    T x, y, z, w;
};
    
struct Voxel {
    u32 color;
    Vec4<i16> coordinates;
};

static u16 SLICE_WIDTH = 256;
static u16 SLICE_HEIGHT = 256;
static u16 SLICE_COUNT = 1;
static u32 SLICE_VOXEL_COUNT;
static u32 DATA_VOXEL_COUNT;
static bool READ_ID_BEFORE_DOT = false;

int main(int argc, char** argv) {
    
    int i = 2;
    while(i < argc) {
        const std::string name = argv[i];
        if(name.find("slice-width:") == 0) {
           SLICE_WIDTH = std::stoi(name.substr(12));
        } else if(name.find("slice-height:") == 0) {
           SLICE_HEIGHT = std::stoi(name.substr(13));
        } else if(name.find("slice-count:") == 0) {
           SLICE_COUNT = std::stoi(name.substr(12));
        } else if(name.find("read-id-before-dot") == 0) {
            READ_ID_BEFORE_DOT = true;
        }
        i++;
    }
    
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(argv[1])) != NULL) {
        printf("Scanning %s folder...\n", argv[1]);
        SLICE_VOXEL_COUNT = SLICE_WIDTH * SLICE_HEIGHT;
        DATA_VOXEL_COUNT = SLICE_VOXEL_COUNT * SLICE_COUNT;
        u32* const data = new u32[DATA_VOXEL_COUNT];
        
        try {
            u16 ns = 0;
            while((ent = readdir(dir)) != NULL) {
                std::string name = ent->d_name;
                if(name != ".." && name != ".") {
                    std::size_t pos = name.find_last_of(".");
                    if(pos != std::string::npos) {
                        std::string filename = argv[1];
                        filename += "/" + name;
                        FILE* const f = fopen(filename.c_str(), "rb");
                        if(f != NULL) {
                            std::string sid;
                            if(READ_ID_BEFORE_DOT) {
                                sid = name.substr(0, pos);
                            } else sid = name.substr(pos + 1);
                            const u32 id = std::stoi(sid);
                            const u32 offset = id * SLICE_VOXEL_COUNT;
                            printf("Slice id: %u. File (%s) loaded.\n", id, name.c_str());
                            fseek(f, 18, SEEK_SET);
                            fread(&(data[offset]), sizeof(u32), SLICE_VOXEL_COUNT, f);
                            fclose(f);
                            ns++;
                        }
                    }
                }
            }
            printf("%u files read\n", ns);
            closedir(dir);
        } catch(const std::invalid_argument& e) {
            printf("Retry by %s read-id-before-dot option.", READ_ID_BEFORE_DOT ? "removing" : "adding");
            return 0;
        }
        
        const u16 SLICE_HALF_WIDTH = SLICE_WIDTH / 2;
        const u16 SLICE_HALF_HEIGHT = SLICE_HEIGHT / 2;
        const u16 SLICE_HALF_COUNT = SLICE_COUNT / 2;
        
        FILE* const f = fopen("voxels.bin", "wb");
        if(f != NULL) {
            u16 k = 0;
            while(k < SLICE_COUNT) {
                u16 i = 0;
                while(i < SLICE_WIDTH) {
                    u16 j = 0;
                    while(j < SLICE_HEIGHT) {
                        const u32 color = data[i + j * SLICE_WIDTH + k * SLICE_VOXEL_COUNT];
                        if((color & 0xFF000000) != 0) {
                            const u32 a = (u8)(color >> 24);
                            const u32 r = (u8)(color >> 16);
                            const u32 g = (u8)(color >> 8);
                            const u32 b = (u8)color;
                            
                            const Voxel voxel = {
                                r << 24 | g << 16 | b << 8 | a,
                                {
                                    (i16)(i - SLICE_HALF_WIDTH),
                                    (i16)(j - SLICE_HALF_HEIGHT),
                                    (i16)(k - SLICE_HALF_COUNT),
                                    1
                                }
                            };
                            fwrite(&voxel, sizeof(Voxel), 1, f);
                        }
                        j++;
                    }
                    i++;
                }                
                printf("\rWrite %u / %u", k + 1, SLICE_COUNT);
                k++;
            }
            fclose(f);
        }
        delete [] data;
    }
    return 0;
}
