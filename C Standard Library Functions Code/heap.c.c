/*
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm. 
 */
#include <stdio.h> // printf

/*
 * The following global variables are used to simulate memory allocation
 * Cortex-M's SRAM space.
 */
// Heap
char array[0x8000];            // simulate SRAM: 0x2000.0000 - 0x2000.7FFF
int heap_top   = 0x20001000;   // the top of heap space
int heap_bot   = 0x20004FE0;   // the address of the last 32B in heap
int max_size   = 0x00004000;   // maximum allocation: 16KB = 2^14
int min_size   = 0x00000020;   // minimum allocation: 32B = 2^5

// Memory Control Block: 2^10B = 1KB space
int mcb_top    = 0x20006800;   // the top of MCB
int mcb_bot    = 0x20006BFE;   // the address of the last MCB entry
int mcb_ent_sz = 0x00000002;   // 2B per MCB entry
int mcb_total  = 512;          // # MCB entries: 2^9 = 512 entries

/*
 * Convert a Cortex SRAM address to the corresponding array index.
 * @param  sram_addr address of Cortex-M's SRAM space starting at 0x20000000.
 * @return array index.
 */
int m2a( int sram_addr ) {
	// TODO: part 1 - add comment to each of the following line of code
	int index = sram_addr - 0x20000000; // index array(heap) equal to the address of cortex-M's SRAM - starting space at 0x20000000 
	return index; //return array index
}

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */ 
int a2m( int array_index ) {
	//  TODO: part 1 - add comment to each of the following line of code
	return array_index + 0x20000000; //return the cortex sram address by adding the starting address of Cortex-M's Sram address and array index
}

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray( ) {
	printf( "memory ............................\n" );
	// T TODO: part 1 - add comment to each of the following line of code
	for ( int i = 0; i < 0x8000; i+=4 ) //loop has to be less than the array of heap size 0x8000, +=4 is adding 4bits each loop
		if ( a2m( i ) >= 0x20006800 ) //if Cortex SRAM address >= than 0x20006800/thetopofMCB 
			printf( "%x = %x(%d)\n", a2m( i ), *(int *)&array[i], *(int *)&array[i] ); // print out cortex address in hexadecimal equal to the address inside array SRAM in hex following by decimal
}

/*
 * _ralloc is _kalloc's helper function that is recursively called to
 * allocate a requested space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  size  the size of a requested memory space
 * @param  left_mcb_addr  the address of the left boundary of MCB entries to examine
 * @param  right_mcb_addr the address of the right boundary of MCB entries to examine
 * @return the address of Cortex-M's SRAM space. While the computation is
 *         made in integers, cast it to (void *). The gcc compiler gives
 *         a warning sign:
                cast to 'void *' from smaller integer type 'int'
 *         Simply ignore it.
 */
void *_ralloc( int size, int left_mcb_addr, int right_mcb_addr ) {
	// initial parameter computation
	//  TODO: part 1 - add comment to each of the following line of code
	int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr  + mcb_ent_sz; // entire mcb addr space equal to address of right boundary of MCB(mcb_bot) - left boundary of MCB(mcb_top) + MCB entry(2B per entry)
	int half_mcb_addr_space = entire_mcb_addr_space / 2; //half mcb addr space equal to entire mcb space divided by 2
	int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space; //midpoint mcb addr space equal to leftboundary(mcb_top) plus half mcb address space
	int heap_addr = 0; //declare heap address equal to 0
	int act_entire_heap_size = entire_mcb_addr_space * 16; //active entire heap size equal to entire memory control block address space multiply by 16bits or 2btyes
	int act_half_heap_size = half_mcb_addr_space * 16;  //active half heap size equal to half mcb addr space multiply by 16bits or 2btyes(entry for each storing/reading data)

	// base case
	//  TODO: part 1 - add comment to each of the following line of code
	if ( size <= act_half_heap_size ) { // if size less or equal than active half heap size which means it can be fit in half of that active memory
		void* heap_addr = _ralloc( size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz ); //call recursive ralloc to split the memory into half of midpoint of mcb to the mcb entry(2B per entry) checking the left boundary
		if ( heap_addr == 0 ) {  //if heap address(memory) is null which mean that address has not enough space to fill
			return _ralloc( size, midpoint_mcb_addr, right_mcb_addr ); //return by calling ralloc recursively to another half memory calling midpoint and right boundary of memory now to check right boundary
		}
		if ( ( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 ) //if LSB of mcb AND operation with 0x01 is equal to 0 then address is available to fit in the memory
			*(short *)&array[ m2a( midpoint_mcb_addr ) ] = act_half_heap_size; // write the active half heap size(update the right half) into the array index of midpoint mcb address (short means 16bits pointers)
		return heap_addr; //return heap address
	}
	else {
		if ( ( array[ m2a( left_mcb_addr ) ] & 0x01 ) != 0 ) { //if lsb of array index of left mcb boundary AND with 0x01 not equal 0 then block is unavailable or in use
			return 0; //return null
		}
		else {
			if ( *(short *)&array[ m2a( left_mcb_addr ) ] < act_entire_heap_size ) // checking if the array sram space of index the left mcb boundary address less than active entire heap size
				return 0; // return null
			 //store the array index of left mcb addr equal to the active entire heap size and set it to in use LSB to 1 with or operation 0x01 
			return (void *)( heap_top + ( left_mcb_addr - mcb_top ) * 16 ); //return the address pointer of first address of heap plus (left mcb boundary minus first mcb adderss) multiply by 16bits
		}
	}
	return 0; // return NULL
}

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree( int mcb_addr ) {
	//  TODO: part 1 - add comment to each of the following line of code
	short mcb_contents = *(short *)&array[ m2a( mcb_addr ) ]; //mcb content represent the used bit content of the mcb address that need to be free, so it set it equal to the array of the mcb address call from kfree 
	int mcb_index = mcb_addr - mcb_top; // to find mcb index in array heap is to the mcb_address get from kfree minus the first mcb adress
	int mcb_disp = ( mcb_contents /= 16 ); // mcb displacement represent the amount in which index should be free which equal to the mcb content /= 16. we divide with 16bits because we use 16bits/2B space to store/read
	int my_size = ( mcb_contents *= 16 ); // my_size equal to the amount of mcb content and multiply by 16 to get the actual size 

	// mcb_addr's used bit was cleared
	*(short *)&array[ m2a( mcb_addr ) ] = mcb_contents;
	
	//  TODO: part 1 - add comment to each of the following line of code
	if ( ( mcb_index / mcb_disp ) % 2 == 0 ) {  // check if the buddy is on the left or right boundary, if remainder is 0, the buddy is on lower layers
		if ( mcb_addr + mcb_disp >= mcb_bot )
			return 0; // my buddy is beyond mcb_bot!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr + mcb_disp ) ]; //check its right buddy  
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {  //if lsb is 0 then it is indicating the availability 
				mcb_buddy = ( mcb_buddy / 32 ) * 32; // move the upper buddy partner
				if ( mcb_buddy == my_size ) { // if upper buddy equal to the actual size content to free
					*(short *)&array[ m2a( mcb_addr + mcb_disp ) ] = 0; // zero-reinitialize 
					my_size *= 2; // double the size content
					*(short *)&array[ m2a( mcb_addr ) ] = my_size; // make sure that it shows the availability size to the next layer
					return _rfree( mcb_addr ); // recursively check the buddy at higher layers
				}
			}
		}
	}
	else {  
		if ( mcb_addr - mcb_disp < mcb_top ) //check if the address and mcb displace need to clear inside the memory block or not
			return 0; // my buddy is below mcb_top!
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr - mcb_disp ) ]; //check its left buddy 
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {  //if lsb is 0 then it is indicating the availability 
				mcb_buddy = ( mcb_buddy / 32 ) * 32; // move the upper buddy partner
				if ( mcb_buddy == my_size ) { // if upper buddy equal to the actual size content to free
					*(short *)&array[ m2a( mcb_addr ) ] = 0; // zero-reinitialize 
					my_size *= 2; // double the size content
					*(short *)&array[ m2a( mcb_addr - mcb_disp ) ] = my_size; // make sure that it shows the availability size to the next layer
					return _rfree( mcb_addr - mcb_disp ); // recursively check the buddy at lower layers
				}
			}
		}
	}
  
  return mcb_addr; //return deallocated mcb address 
}

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit( ) {
	//  TODO: part 1 - add comment to each of the following line of code
	for ( int i = 0x20001000; i < 0x20005000; i++ ) // i = 0x20001000(top of heap space) loop to the bottom of heap at 0x20004FE0
		array[ m2a( i ) ] = 0; // set each heap to 0 

		*(short *)&array[ m2a( mcb_top ) ] = max_size; // set the top of mcb (first address of mcb) to be the maximum allocation size (the entire space is available) 

	for ( int i = 0x20006804; i < 0x20006C00; i += 2 ) { // from i = 0x20006804 of mcb loop to the end of mcb entry, i+= is 2B per mcb entry
		array[ m2a( i ) ] = 0; // set each index of mcb address to zero initialized
		array[ m2a( i + 1) ] = 0; // set the next index of mcb address to zero initialized
	}
}

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc( int size ) {
  return _ralloc( size, mcb_top, mcb_bot );
}

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree( void *ptr ) {
	//  TODO: part 1 - add comment to each of the following line of code
	int addr = (int )ptr; //convert ptr addr to int addr

	if ( addr < heap_top || addr > heap_bot ) //if address less than first address of heap or greater than the bottom/last address of heap
		return NULL; //return null
	int mcb_addr =  mcb_top + ( addr - heap_top ) / 16; // mcb address to free equal to the sum of first mcb address and (address minus the heap top)

	if ( _rfree( mcb_addr ) == 0 ) // call rfree to check at mcb address if it is equal to 0(already null)
		return NULL; // return null
	else
		return ptr; // return ptr(the address of this deallocated space)
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc( int size ) {
	static int init = 0;
	if ( init == 0 ) {
		init = 1;
		_kinit( );
	}
	return _kalloc( size );
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free( void *ptr ) {
	return _kfree( ptr );
}
