/*
 * ConcurrentHopscotchHashSet.h
 *
 *  Created on: 23 дек. 2017 г.
 *      Author: Рома
 */

#ifndef CONCURRENTHOPSCOTCHHASHSET_H_
#define CONCURRENTHOPSCOTCHHASHSET_H_

template <class KEY, class DATA>
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
		unsigned int volatile lock;
		unsigned int volatile timestamp;
		void lock();
		void unlock();
	};
	Bucket* segments_ary[MAX_SEGMENTS][];
	unsigned int segment_mask;
	unsigned int bucket_mask;
	void ﬁnd_closer_free_bucket (Bucket** free_bucket, int* free_distance);
 public:
	bool contains( KEY* key);
	DATA* add( KEY* key, DATA* data);
	DATA* remove( KEY* key);
};

#endif /* CONCURRENTHOPSCOTCHHASHSET_H_ */
