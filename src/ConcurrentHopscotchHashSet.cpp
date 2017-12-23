/*
 * ConcurrentHopscotchHashSet.cpp
 *
 *  Created on: 23 дек. 2017 г.
 *      Author: Рома
 */

#include "ConcurrentHopscotchHashSet.h"

ConcurrentHopscotchHashSet::ConcurrentHopscotchHashSet() {
	// TODO Auto-generated constructor stub

}

ConcurrentHopscotchHashSet<KEY, DATA>::~ConcurrentHopscotchHashSet() {
	// TODO Auto-generated destructor stub
}

template <class KEY, class DATA>
DATA* ConcurrentHopscotchHashSet<KEY, DATA>::add(KEY* key, DATA* data)
{
	unsigned int hash = CalcHashFunc( key−>hashCode());
	unsigned int iSegment = hash & segment_mask;
	unsigned int iBucket = hash & bucket_mask;
	Bucket* start_bucket = segments_ary[iSegment][iBucket];
	start_bucket->lock();
	if (contains(key)) {
		DATA* rc = key’s data;
		start_bucket −>unlock();
		return rc;  }
	Bucket* free_bucket = start_bucket;
	int free_distance = 0;
	for (; free_distance < ADD_RANGE; ++free_distance)
	{
		if (NULL == free_bucket−> key && NULL == ATOMIC_CAS(&(free_bucket−> key), NULL, BUSY)))
		  break;
		  ++free_bucket;
	}
	if ( free_distance < ADD_RANGE)
	{
		do {
			if (free_distance < HOP_RANGE)
			{
				start_bucket −> hopInfo |= (1 << free_distance);
				free_bucket−> data = data;
				free_bucket−> key = key;
				start_bucket −>unlock();
				return NULL;
			}
			ﬁnd_closer_free_bucket (&free_bucket, &free_distance );
		} while (NULL != free_bucket);
	}
	start_bucket->unlock();
	resize();
	return add(key, data);
}

template <class KEY, class DATA>
bool ConcurrentHopscotchHashSet<KEY, DATA>::contains( KEY* key)
{
	unsigned int hash = CalcHashFunc( key−>hashCode());
	unsigned int iSegment = hash & segment_mask;
	unsigned int iBucket = hash & bucket_mask;
	Bucket* start_bucket = segments_ary[iSegment][iBucket];
	unsigned int try_counter = 0;
	unsigned int timestamp;
	do {
		timestamp = start_bucket−> timestamp;
		unsigned int hop_info = start_bucket−> hop_info;
		for each check_bucket in hop_info
		{
			if (key.equal(check_bucket−> key))
				return true;
		}
		++try_counter;
	} while (timestamp != start_bucket−> timestamp && try_counter < MAX TRIES);
	if (timestamp != start_bucket−> timestamp )
	{
		Bucket* check_bucket = start_bucket;
		for (int i=0; i < HOP_RANGE; ++i)
		{
			if (key.equal(check_bucket−> key))
				return true;
			++check_bucket;
		}
	}
	return false ;
}

template <class KEY, class DATA>
void ConcurrentHopscotchHashSet<KEY, DATA>::ﬁnd_closer_free_bucket (Bucket** free_bucket, int* free_distance)
{
	Bucket* move_bucket = *free_bucket − (HOP_RANGE − 1);
	for (int free_dist = (HOP_RANGE − 1); free_dist > 0; −−free_dist)
	{
		unsigned int start hop_info = move_bucket−>hop_info;
		int move_free_distance = −1;
		unsigned int mask = 1;
		for (int i = 0; i < free_dist ; ++i, mask <<= 1)
		{
			if (mask & start_hop_info)
			{
				move_free_distance = i;
				break;
			}
		}
		if (−1 != move_free_distance)
		{
			move_bucket−>lock();
			if ( start_hop_info == move_bucket−>hop_info)
			{
				Bucket* new_free_bucket = move_bucket + move_free_distance;
				move_bucket−>hop_info |= (1 << free_dist);
				free_bucket−> data = new_free_bucket−> data;
				free_bucket−> key = new_free_bucket−> key;
				++(move_bucket−> timestamp);
				new_free_bucket−> key = BUSY;
				new_free_bucket−> data = BUSY;
				move_bucket−>hop_ info &= ˜(1 << move_free_distance);
				*free_bucket = new_free_bucket;
				*free_distance −= free_dist;
				move_bucket−>unlock();
				return;
			}
			move_bucket−>unlock();
		}
		++move_bucket;
	}
	*free_bucket = NULL;
	*free_distance = 0;
}

template <class KEY, class DATA>
DATA* ConcurrentHopscotchHashSet<KEY, DATA>::remove( KEY* key)
{
	unsigned int hash = CalcHashFunc( key−>hashCode() );
	unsigned int iSegment = hash & segment_mask; 4 unsigned int iBucket = hash & bucket_mask;
	Bucket* start_bucket = segments_ary[iSegment][iBucket];
	start_bucket −>lock();

	unsigned int hop_info = start_bucket−>hop_info;
	for( each check indx in hop_info )
	{
		Bucket* check_bucket = start_bucket + check_indx;
		if (key.equal(check_bucket−> key))
		{
			DATA* rc = check_bucket−> data;

			check_bucket−> key = NULL;
			check_bucket−> data = NULL;
			start_bucket −> hop_info &= ˜( 1 << check indx );
			start_bucket −>unlock();
			return rc;
		}
	}
	start_bucket −>unlock();
	return NULL;
}
