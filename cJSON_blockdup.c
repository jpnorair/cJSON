/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#include "cJSON.h"
#include "cJSON_blockdup.h"


/** JPN 
  */
#define _ALIGN          (sizeof(void*))
#define _DO_ALIGN(LEN)  ( (LEN+(_ALIGN-1)) & ~(_ALIGN-1) )



// Interior functions Prototypes
size_t cJSON_GetObjectSize_Loop(const cJSON *item, cJSON_bool recurse, size_t size);
CJSON_PUBLIC(cJSON *) cJSON_BlockDup_Loop(void** dst, int* rembytes, const cJSON *item, cJSON_bool recurse);
static unsigned char* cJSON_strdup_block(void** dst, int* rembytes, const unsigned char* string);







size_t cJSON_GetObjectSize(const cJSON *item) {
    return cJSON_GetObjectSize_Loop(item, true, 0);
}



CJSON_PUBLIC(cJSON *) cJSON_BlockDup(void* dst, size_t max, const cJSON *item) {
    int rembytes = (int)max;
//fprintf(stderr, "cJSON_BlockDup(): dst = %016llx\n", (uint64_t)dst);   
    return cJSON_BlockDup_Loop(&dst, &rembytes, item, true);
}





/// Interior Function Implementations

size_t cJSON_GetObjectSize_Loop(const cJSON *item, cJSON_bool recurse, size_t size) {
    cJSON *child = NULL;
    size_t strsz;

    /* Bail on bad ptr */
    if (item != NULL) {
        /* Top Level Size */
        size += _DO_ALIGN(sizeof(cJSON));
//fprintf(stderr, "new cJSON\n");   
//fprintf(stderr, "size=%zu (+%zu : %zu)\n", size, sizeof(cJSON), _DO_ALIGN(sizeof(cJSON)));

        /* Size over all vars */
        if (item->valuestring) {
            strsz   = strlen(item->valuestring) + sizeof(char);
            size   += _DO_ALIGN(strsz);
//fprintf(stderr, "item->valuestring: %s\n", item->valuestring);   
//fprintf(stderr, "size=%zu (+%zu : %zu)\n", size, strsz, _DO_ALIGN(strsz));
        }
        if (item->string) {
            if (item->type & cJSON_StringIsConst) {
                size += _DO_ALIGN(sizeof(const char*));
//fprintf(stderr, "item->type: ***const string***\n");
//fprintf(stderr, "size=%zu (+%zu : %zu)\n", size, sizeof(const char*), _DO_ALIGN(sizeof(const char*)));
            }
            else {
                strsz   = strlen(item->string) + sizeof(char);
                size   += _DO_ALIGN(strsz);
//fprintf(stderr, "item->string: %s\n", item->string);     
//fprintf(stderr, "size=%zu (+%zu : %zu)\n", size, strsz, _DO_ALIGN(strsz));
            }
        }
    
        /* If non-recursive, then we're done! */
        /* If recursive, Walk the ->next chain for the child. */
        if (recurse) {
            child = item->child;
            while (child != NULL) {
                size    = cJSON_GetObjectSize_Loop(child, true, size); /* Duplicate (with recurse) each item in the ->next chain */
                child   = child->next;
            }
        }
    }
    
    return size;
}





static unsigned char* cJSON_strdup_block(void** dst, int* rembytes, const unsigned char* string) {
    size_t length;
    size_t size;
    unsigned char* str;

//fprintf(stderr, "cJSON_strdup_block(%016llx, %i, %s)\n", (uint64_t)*dst, *rembytes, string);   

    if ((string == NULL) || (*dst == NULL))  {
        return NULL;
    }

    length      = strlen((const char*)string) + sizeof(char);
    size        = _DO_ALIGN(length);
    *rembytes  -= size;
    if (*rembytes < 0) {
        return NULL;
    }
    
    bzero(*dst, size);
    memcpy(*dst, string, length);
    str     = *dst;
    *dst   += size;

    return str;
}




CJSON_PUBLIC(cJSON *) cJSON_BlockDup_Loop(void** dst, int* rembytes, const cJSON *item, cJSON_bool recurse) {
    cJSON *newitem = NULL;
    cJSON *child = NULL;
    cJSON *next = NULL;
    cJSON *newchild = NULL;
    void* base;
    size_t size;

//fprintf(stderr, "cJSON_BlockDup_Loop(%016llx, %i, %016llx, %i)\n", (uint64_t)*dst, *rembytes, (uint64_t)item, recurse);   

    /* Bail on bad ptr */
    /* The dst input is guaranteed to be non-null, by design */
    if ((item == NULL) || (*dst == NULL)) {
        goto fail;
    }
    
    /* Create new item */
    size        = _DO_ALIGN(sizeof(cJSON));
    *rembytes  -= size;
    if (*rembytes < 0) {
        goto fail;
    }
    
    newitem = (cJSON*)*dst;
    bzero(*dst, size);
    *dst += size;
//fprintf(stderr, "newitem: %016llx : %zu\n", (uint64_t)newitem, size);   
//fprintf(stderr, "dst: %016llx\n", (uint64_t)*dst);   
        
    /* Copy over all vars */
    newitem->type           = item->type & (~cJSON_IsReference);
    newitem->valueint       = item->valueint;
    newitem->valuedouble    = item->valuedouble;
    
    if (item->valuestring) {
        newitem->valuestring = (char*)cJSON_strdup_block(dst, rembytes, (unsigned char*)item->valuestring);
//fprintf(stderr, "newitem->valuestring: %016llx (%s)\n", (uint64_t)newitem->valuestring, newitem->valuestring);   
//fprintf(stderr, "dst: %016llx\n", (uint64_t)*dst);   
        if (!newitem->valuestring) {
            goto fail;
        }
    }
    if (item->string) {
        if (item->type&cJSON_StringIsConst) {
            newitem->string = item->string;
//fprintf(stderr, "newitem->string: ***const*** %016llx (%s)\n", (uint64_t)newitem->string, newitem->string);   
//fprintf(stderr, "dst: %016llx\n", (uint64_t)*dst);   
        }
        else {
//fprintf(stderr, "cJSON_strdup_block()\n");   
            newitem->string = (char*)cJSON_strdup_block(dst, rembytes, (unsigned char*)item->string);
//fprintf(stderr, "newitem->string: %016llx (%s)\n", (uint64_t)newitem->string, newitem->string);   
//fprintf(stderr, "dst: %016llx\n", (uint64_t)*dst);   
        }
        if (newitem->string == NULL) {
            goto fail;
        }
    }
    
    /* If non-recursive, then we're done! */
    /* If recursive, Walk the ->next chain for the child. */
    if (recurse) {
        ///@note recursion may need to be modified for loop, depending on how rembytes fits on the stack.
        
        child = item->child;
        while (child != NULL) {
//fprintf(stderr, "cJSON_BlockDup_Loop()\n");   
            newchild = cJSON_BlockDup_Loop(dst, rembytes, child, true);
//fprintf(stderr, "newchild: %016llx\n", (uint64_t)newchild);   
//fprintf(stderr, "dst: %016llx\n", (uint64_t)*dst); 
            
            if (newchild == NULL) {
                goto fail;
            }
            
            if (next != NULL) {
                /* If newitem->child already set, then crosswire ->prev and ->next and move on */
                next->next      = newchild;
                newchild->prev  = next;
                next            = newchild;
            }
            else {
                /* Set newitem->child and move to it */
                newitem->child = newchild;
                next = newchild;
            }
            
            child = child->next;
        }
    }

    return newitem;

fail:
    /// Don't need to delete the interim cJSON object because no memory was allocated.
    return NULL;
}


