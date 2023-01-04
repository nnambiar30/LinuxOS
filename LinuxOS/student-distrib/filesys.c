#include "sysCalls.h"
#include "lib.h"
#include "filesys.h"

boot_block_t *origin;
data_block_t *first_data_block; 
dentry_t* dentry_obj;
uint32_t inode_num;
inode_t *root_inode;


/* int32_t file_init(boot_block_t *boot);
 * Inputs: boot_block_t *boot
 * Return Value: 0
 * Function: inits filesystem */
int32_t file_init(boot_block_t *boot){
    origin = boot;                                              // ptr to boot block
    root_inode = (inode_t*)(origin + 1);                                  // ptr to first inode
    inode_num = origin->inode_count;                            // holds total number of inodes 
    dentry_obj = &(origin->direntries[62]);                      // ptr to 62nd dentry (empty dentry)    
    first_data_block = (data_block_t*)(origin + inode_num + 1); //contains ptr to the first data block
    return 0;
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
 * Inputs: const uint8_t* fname, dentry_t* dentry
 * Return Value: -1 if not found, 0 if found
 * Function: reads dentry by index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if(origin->direntries[index].inode_num < 0 || origin->direntries[index].inode_num > ENTRY_NUM){
        return -1;
    }
    memcpy((dentry), &(origin->direntries[index]), sizeof(dentry_t));
    return 0;
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
 * Inputs: const uint8_t* fname, dentry_t* dentry
 * Return Value: -1 if not found, 0 if found
 * Function: reads dentry by name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    int idx = 0; //file index

    if(strlen((int8_t*)fname) > FILENAME_LEN || fname == NULL) return -1; //check if the filename is not NULL and is at max 32 characters

    //putc(strlen((int8_t*)fname));

    /* go through each dentry to check if the filename exists in the dentry, then copy to dentry object and return*/
    for(idx = 0; idx < ENTRY_NUM; idx++){
        if(strncmp((int8_t*)origin->direntries[idx].filename, (int8_t*)fname, 32) == 0){ 
            return read_dentry_by_index(idx, dentry);
        }
    }
    
    
    return -1; //return -1 when file name does not exist
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
 * Inputs: uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
 * Return Value: number of bytes read
 * Function: reads data from a given file(inode) */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    inode_t* inode_ptr;         //ptr for inode
    data_block_t* data_blocks;  //array of data blocks
    data_block_t* data_block;   // pointer to a data block
    uint32_t db_offset, i, block_len;

    /*check if the inode passed in is within bounds*/
    if(inode >= origin->inode_count){
        return -1;
    }
    inode_ptr = get_inode(inode);
    block_len = (inode_ptr->length / 4096) + 1; //number of data blocks for the inode
    db_offset = offset / 4096; //get offset to data block

    /*check to make sure the offset is not out of bounds of the # of data blocks*/
    if(db_offset >= block_len){
        return -1;
    }

    
    offset %= 4096; //get offset inside data block
    data_blocks = (data_block_t*)((origin->inode_count * sizeof(inode_t)) + (uint8_t*)get_inode(0)); // pointer to data blocks for the inode
    data_block = &data_blocks[inode_ptr->data_block_num[db_offset]];  //pointer to the specified data block that we are starting to get data from
    if(inode_ptr->data_block_num[db_offset] >= origin->data_count){
        return -1;
    }
    /*loop through the length of the file(size) and read*/
    for(i = 0; i < length; i++){
        buf[i] = data_block->data[(i+offset) % 4096];
        if((i+offset+1) % 4096 == 0){
            db_offset++;
            if(db_offset == block_len){
                return i;
            }
            /*bounds checks*/
            else if(db_offset > block_len){
                return -1;
            }
            /*bounds checks*/
            if(inode_ptr->data_block_num[db_offset] >= origin->data_count){
                return -1;
            }
            /*reset data block pointer with new db offset*/
            data_block = &data_blocks[inode_ptr->data_block_num[db_offset]];
            offset = 0;
        }
    }
    return i;
} 


/* inode_t* get_inode(uint32_t inode)
 * Inputs: uint32_t inode
 * Return Value: inode_t*
 * Function: Helper function that given an inode number, gets a pointer to the specified inode */
inode_t* get_inode(uint32_t inode){
    return (inode_t*)((inode * sizeof(inode_t)) + ((uint8_t*)origin + 4096));
}

/* uint32_t get_file_len(dentry_t* dentry)
 * Inputs: dentry_t* dentry
 * Return Value: length
 * Function: Helper function to return the length of a file */
uint32_t get_file_len(dentry_t* dentry){
    return ((inode_t *)(root_inode + dentry->inode_num))->length;
}

/* int32_t open_f(const uint8_t* filename);
 * Inputs: const uint8_t* filename
 * Return Value: -1 if not found, 0 if found
 * Function: opens file */
int32_t open_f(const uint8_t* filename){
    if(filename == NULL) return -1;
    return read_dentry_by_name(filename, dentry_obj); //open file by reading the filename
}

/* int32_t write_f(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: -1 
 * Function: writes file */
int write_f(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}

/* int32_t close_f(int32_t fd);
 * Inputs: int32_t fd
 * Return Value: -1 
 * Function: closes file */
int32_t close_f(int32_t fd){
    return 0;
}

/* int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: -1 if not found, 0 if found
 * Function: reads file */
int32_t read_f(int32_t fd, void *buf, int32_t nbytes){
    pcb_t* cur_pcb = get_pcb_from_pid(pid);
    uint32_t bread = read_data(cur_pcb->fd_arr[fd].inode_num, cur_pcb->fd_arr[fd].file_position, buf, nbytes); //read file data in read file function
    cur_pcb->fd_arr[fd].file_position += bread;
    return bread;
}

/* int32_t open_d(const uint8_t* filename);
 * Inputs: const uint8_t* filename
 * Return Value: -1 if not found, 0 if found
 * Function: opens directory */
int32_t open_d(const uint8_t* filename){
    if(filename == NULL) return 0;
    read_dentry_by_name(filename, dentry_obj);
    /*check if the stored dentry information is a directory type*/
    if(dentry_obj->filetype ==1){
        return 0;
    }
    return -1; //return -1 if file is not a directory
}

/* int32_t write_d(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: -1 
 * Function: writes directory */
int32_t write_d(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}

/* int32_t close_d(int32_t fd);
 * Inputs: int32_t fd
 * Return Value: -1 
 * Function: closes directory */
int32_t close_d(int32_t fd){
    return 0;
}

/* int32_t read_d(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: -1 if not found, 0 if found
 * Function: reads directory */
int32_t read_d(int32_t fd, void *buf, int32_t nbytes){
    if(buf == NULL) return -1;         // null check
    int j = 0;
    pcb_t* cur_pcb = get_pcb_from_pid(pid);
    uint32_t file_to_read = cur_pcb->fd_arr[fd].file_position++;

    
    if(file_to_read < 63){
        if((int8_t*)!strncmp("\0", (int8_t*)origin->direntries[file_to_read].filename, sizeof(origin->direntries[file_to_read].filename))) return 0;
        /*loop to check how many bytes is contained in the filename*/
        for(j =0; j < 32; j++){
            if(origin->direntries[file_to_read].filename[j] == '\0'){
                nbytes = j;
                break;
            }
        }
        memcpy(buf, origin->direntries[file_to_read].filename, sizeof(origin->direntries[file_to_read].filename)); //copy filename to buffer
        return nbytes;
    }

    return 0;
}


/* int32_t get_filetype_from_inode(uint32_t inode_num);
 * Inputs: uint32_t inode_num
 * Return Value: return filetype, -1 if inode_num DNE
 * Function: helper function to return filetype of a file given the inode number */
int32_t get_filetype_from_inode(uint32_t inode_num){
    int i;
    /*check which direntry pertains to the inode_num passed in */
    for(i = 0; i< 63; i++){
        if(inode_num == origin->direntries[i].inode_num){
            /*return the filetype*/
            return origin->direntries[i].filetype;
        }
    }
    return -1; // inode_num does not exist so return -1
}
