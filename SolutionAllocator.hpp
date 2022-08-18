#include <stdlib.h>
#include"ChunkAlloc.hpp"

template<
	class T,          // This is the type we are allocating.  You can assume this is less than or equal to 4kB
	size_t ALIGNMENT  // The alignment at which we much allocate the objects.  You can assume this is less than or equal to 4kB
	> 
class SolutionAllocator {
	std::vector<uint8_t*> chunks; // We store everything we allocated so we can clean up in the destructor.
    std::vector<T*> free_list;
    size_t section_size;
    size_t num_T;
    size_t max_num_T;
    uint8_t* curr;
    T* new_alloc;
public:
 	typedef T ItemType; // This will make T available as SolutionAllocator::ItemType 
	static const size_t Alignment = ALIGNMENT;  // Likewise, we can access the alignment as SolutionAllocator::Alignment
    static const size_t size = sizeof(T);
	
	SolutionAllocator() {
        if(size % Alignment == 0) {
            section_size = size;
        }
        else {
            section_size = ((size / Alignment) + 1) * Alignment;
        }
        
        num_T = 0;
        max_num_T = CHUNK_SIZE / section_size;
        curr = NULL;
        new_alloc = NULL;
    }
	
    T * alloc() {  
        if(free_list.size() > 0) {
            new_alloc = free_list.back();
            
            T* recycle = new_alloc;
            
            memset(recycle, 0, section_size);
            
            free_list.pop_back();
        }
        else if(chunks.size() == 0 || num_T == max_num_T) {
            void* p = alloc_chunk();
            new_alloc = reinterpret_cast<T*>(p);
            curr = reinterpret_cast<uint8_t*>(p);
            chunks.push_back(curr);
            curr += section_size;
            num_T = 1;
        }
        else {
            new_alloc = reinterpret_cast<T*>(curr);
            curr += section_size;
            ++num_T;
        }
   
        new (new_alloc) T;
        return new_alloc;
	}
	
	void free(T * p) {
		free_list.push_back(p);
	}

	~SolutionAllocator() {
		for(auto & p: chunks) { 
			free_chunk(p); // Return everything that still allocated.
		}
	}
};

template<class T, size_t ALIGNMENT> 
const size_t SolutionAllocator<T, ALIGNMENT>::Alignment;
