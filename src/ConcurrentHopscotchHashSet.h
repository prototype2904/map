/*
 * ConcurrentHopscotchHashSet.h
 *
 *  Created on: 23 дек. 2017 г.
 *      Author: Рома
 */

#ifndef CONCURRENTHOPSCOTCHHASHSET_H_
#define CONCURRENTHOPSCOTCHHASHSET_H_
#define BUSY 1
#include <iostream>
#include<atomic>
#include <mutex>


template<class KEY,
		 class DATA,
		 class Hash = std::hash<KEY>,
         class KeyEqual = std::equal_to<KEY>,
         class Allocator = std::allocator<KEY>>
class ConcurrentHopscotchHashSet {
 private :
	static const int HOP_RANGE = 32;
	static const int ADD_RANGE = 256;
	static const int MAX_SEGMENTS = 1024;
	static const int MAX_TRIES = 2;
	struct Bucket {
		unsigned int volatile hop_info;
		KEY* volatile key;
		DATA* volatile data;
		unsigned int volatile _lock;
		unsigned int volatile timestamp;
		std::mutex mtx;
		void lock(){
			mtx.lock();
		}
		void unlock(){
			mtx.unlock();
		}
	};
	Bucket* segments_ary[MAX_SEGMENTS];
	unsigned int segment_mask;
	unsigned int bucket_mask;

	int CAS( KEY* addr, KEY old)
	{
	  if ( *addr != old )
		return 0;

	  *addr = BUSY;
	  return 1;
	}

	void resize(){
		//resize will be in the next year!
	}
	void find_closer_free_bucket(Bucket* free_bucket, int* free_distance){
		Bucket* move_bucket = free_bucket - (HOP_RANGE - 1);
		for(int free_dist = (HOP_RANGE - 1); free_dist > 0; --free_dist)
		{
			unsigned  int start_hop_info = move_bucket->hop_info;
			int move_free_distance = -1;
			unsigned int mask = 1;
			for (int i = 0; i < free_dist ; ++i, mask <<= 1)
			{
				if (mask & start_hop_info)
				{
					move_free_distance = i;
					break;
				}
			}
			if( -1 != move_free_distance)
			{
				move_bucket->lock();
				if(start_hop_info == move_bucket->hop_info)
				{
					Bucket* new_free_bucket = move_bucket + move_free_distance;
					move_bucket->hop_info |= (1 << free_dist);
					free_bucket -> data = new_free_bucket ->data;
					free_bucket -> key = new_free_bucket->key;
					++(move_bucket->timestamp);
//					new_free_bucket->key = BUSY;
//					new_free_bucket->data = BUSY;
					move_bucket->hop_info &= ~(1 << move_free_distance);
					free_bucket = new_free_bucket;
					*free_distance -= free_dist;
					move_bucket -> unlock();
					return;
				}
				move_bucket -> unlock();
			}
			++move_bucket;
		}
		free_bucket = NULL;
		*free_distance = 0;
	}

	int CalcHashFunc(KEY* key)
	{
		std::hash<KEY*> hash_fn;
		return hash_fn(key);
	}
 public:
	DATA* get( KEY* key)
		{
			unsigned int hash = CalcHashFunc(key);
			unsigned int iSegment = hash & segment_mask;
			unsigned int iBucket = hash & bucket_mask;
			Bucket*start_bucket = segments_ary[iSegment] + iBucket;
			unsigned int try_counter = 0;
			unsigned int timestamp;
			do {
				timestamp = start_bucket->timestamp;
				unsigned int hop_info = start_bucket->hop_info;
				for (unsigned int i = 0; i < sizeof(hop_info); i++) {
					Bucket* check_bucket = start_bucket + (1 << ((hop_info >> i) & 1));
					if(key == check_bucket->key)
						return check_bucket->data;
				}
				++try_counter;
			} while(timestamp !=start_bucket -> timestamp && try_counter < MAX_TRIES);
			return NULL ;
		}

	bool contains( KEY* key){
		unsigned int hash = CalcHashFunc(key);
		unsigned int iSegment = hash & segment_mask;
		unsigned int iBucket = hash & bucket_mask;
		Bucket*start_bucket = segments_ary[iSegment]+ iBucket;
		unsigned int try_counter = 0;
		unsigned int timestamp;
		do {
			timestamp = start_bucket->timestamp;
			unsigned int hop_info = start_bucket->hop_info;
			for (unsigned int i = 0; i < sizeof(hop_info); i++) {
				//Лёня так сказал сделать
				//Для каждой корзины из hop_info
				Bucket* check_bucket = start_bucket + (1 << ((hop_info >> i) & 1));
				if(key == check_bucket->key)
					return true;
			}
			++try_counter;
		} while(timestamp !=start_bucket -> timestamp && try_counter < MAX_TRIES);
		if(timestamp != start_bucket -> timestamp)
		{
			Bucket* check_bucket = start_bucket;
			for (int i=0; i < HOP_RANGE; ++i)
			{
				if(key == check_bucket->key)
					return true;
				++check_bucket;
			}
		}
		return false ;
	}
	DATA* add( KEY* key, DATA* data){
		unsigned int hash = CalcHashFunc(key);
		unsigned int iSegment = hash & segment_mask;
		unsigned int iBucket = hash & bucket_mask;
		Bucket*start_bucket = segments_ary[iSegment]+ iBucket;
		start_bucket->lock();
		if (contains(key)) {
			DATA* rc = get(key);
			start_bucket->unlock();
			return rc;
		}
		Bucket* free_bucket = start_bucket;
		int free_distance = 0;
		for (; free_distance < ADD_RANGE; ++free_distance)
		{
			if(NULL == (free_bucket->key) && CAS(free_bucket->key, NULL))
			  break;
			++free_bucket;
		}
		if ( free_distance < ADD_RANGE)
		{
			do {
				if (free_distance < HOP_RANGE)
				{
					start_bucket -> hop_info |= (1 << free_distance);
					free_bucket -> data = data;
					free_bucket -> key = key;
					start_bucket->unlock();
					return NULL;
				}
				find_closer_free_bucket(free_bucket, &free_distance);
			} while (NULL != free_bucket);
		}
		start_bucket->unlock();
		resize();
		return add(key, data);
	}
	DATA* remove( KEY* key){
		unsigned int hash = CalcHashFunc(key);
		unsigned int iSegment = hash & segment_mask;
		unsigned int iBucket = hash & bucket_mask;
		Bucket*start_bucket = segments_ary[iSegment]+ iBucket;
		start_bucket->lock();

		unsigned int hop_info = start_bucket->hop_info;
		for (int i = 0; i < sizeof(hop_info); i++) {
			//Лёня так сказал сделать
			//Для каждой корзины из hop_info
			Bucket* check_bucket = start_bucket + (1 << ((hop_info >> i) & 1));
			if(key == check_bucket->key)
			{
				DATA*rc=check_bucket->data;
				check_bucket->key=NULL;
				check_bucket->data = NULL;
				start_bucket -> hop_info &= ~(1 << i);
				start_bucket ->unlock();
				return rc;
			}
		}
		start_bucket ->unlock();
		return NULL;
	}
	ConcurrentHopscotchHashSet(){
		segment_mask = 10;
		bucket_mask = 10;
		for(int i = 0; i < MAX_SEGMENTS;i++){
			segments_ary[i] = new Bucket();
		}
	}
//	~ConcurrentHopscotchHashSet();
};

#endif /* CONCURRENTHOPSCOTCHHASHSET_H_ */
