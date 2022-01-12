/******************************************************************************
Utility APIs 

Author: Ashish Rao 
Date: 08/Jan/2022

*******************************************************************************/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lpm.h"



extern lpm_node *root;
extern int cache;


void
init_globals(void) 
{
    /* invalidate the cache */
    cache = FALSE;
    
    /* alloc root node */
    lpm_node *new_node = calloc(1,sizeof(lpm_node));
    if (new_node == NULL) {
        printf("calloc failed.\n");
        exit(0);
    }
    new_node->val = -1;
    new_node->nexthop = -1;
    new_node->src_port = -1;
    new_node->left = NULL;
    new_node->right = NULL;
    root = new_node;
}


/* 
 * API to fill the data into the LPM node
 */
void 
fill_data(lpm_node **node, int data, int nh, int port) 
{
    /*
     * VERIFY INPUT DATA
     */
    if (node == NULL || *node == NULL) {
        printf("%s:%d Invalid Input.\n", __FUNCTION__, __LINE__);
        return;
    }
    
    /* fill data */
    (*node)->val = data;
    (*node)->nexthop = nh;
    (*node)->src_port = port;

    return;
}

/*
 * API to convert decimal number to binary
 * and store the binary info in a array
 */
void
dec2bin(unsigned int nw_ip, int *prefix, int size) 
{
    /*
     * VERIFY INPUT DATA
     */
    if (nw_ip == 0 || prefix == NULL || size == 0) {
        printf("%s:%d Invalid PARAMETER. nw_ip %d prefix %p size %d\n",
                     __FUNCTION__, __LINE__, nw_ip, prefix, size);
        return;
    }
    
    //int i = size;   //UNIT TEST
    
    /* convert dec to binary and store in the array */
    while (size > 0) {
        size--;
        if(nw_ip > 0) {
            prefix[size] = nw_ip % 2;
            nw_ip = nw_ip / 2;
        } else {
            prefix[size] = 0;
        }
    }
    
    //UNIT TEST
    /*
    printf("%s:%d size %d prefix ",__FUNCTION__, __LINE__, i);
    for (int j=0; j < i; j++) {
        printf("%d", prefix[j]);
    }
    printf("\n");
    */
}
