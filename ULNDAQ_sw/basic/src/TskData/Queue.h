/**
 * @file  Queue.h
 * @brief 循环队列
 * @details
 * @author Duna
 * @version 1.0.0
 */

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>

#ifndef __QUEUE_H__
#define __QUEUE_H__

template<typename T, bool threadSafe = true>
class Queue
{
private:
	SemaphoreHandle_t mtx;
	T *array;
	int alen;
	volatile int head;
	volatile int tail;
	int l()
	{
		int rtn;
		if(this->tail >= this->head)
		{
			rtn = this->tail - this->head;
		}
		else
		{
			rtn = this->alen + this->tail - this->head;
		}
		return rtn;
	}
public:
	Queue(int alen)
	{
	    if(threadSafe)
	    {
    		this->mtx = xSemaphoreCreateMutex();
            configASSERT(this->mtx);
	    }
		this->array = new T[alen];//(T *)malloc(sizeof(T) * alen);
		this->alen = alen;
		this->head = 0;
		this->tail = 0;
	}
	~Queue()
	{
	    delete this->array;//free((void *)this->array);
        if(threadSafe)
        {
    	    vSemaphoreDelete(mtx);
        }
	}
	void Clear()
	{
	    int ret;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
            configASSERT(ret == pdPASS);
        }
	    this->head = 0;
	    this->tail = 0;
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
	}
	int Capacity()
	{
		return alen - 1;
	}
	int Len()
	{
	    int ret;
		int rtn;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
            configASSERT(ret == pdPASS);
        }
        if(this->tail >= this->head)
		{
			rtn = this->tail - this->head;
		}
		else
		{
			rtn = this->alen + this->tail - this->head;
		}
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
        return rtn;
	}
	bool En(const T &c, bool check = true)
	{
	    int ret;
		bool rtn = false;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
            configASSERT(ret == pdPASS);
        }
        if(!check || l() < alen - 1)
		{
			array[tail] = c;
			if(tail == alen - 1)
			{
				tail = 0;
			}
			else
			{
				tail ++;
			}
	        rtn = true;
		}
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
        return rtn;
	}

	bool EnBlock(const T *data, int len, bool check = true)
	{
	    int ret;
		bool rtn = false;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
            configASSERT(ret == pdPASS);
        }
        if(!check || l() + len <= alen - 1)
		{
        	if(tail + len <= alen)
        	{
            	memcpy((void *)&(array[tail]), (void *)&(data[0]), sizeof(T)*len);
        	}
        	else
        	{
        		int l0 = alen - tail;
        		int l1 = len - l0;
            	memcpy((void *)&(array[tail]), (void *)&(data[0]), sizeof(T)*l0);
            	memcpy((void *)&(array[0]), (void *)&(data[l0]), sizeof(T)*l1);
        	}
        	if(tail + len <= alen - 1)
        		tail += len;
        	else
        		tail = tail + len - alen;
	        rtn = true;
		}
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
        return rtn;
	}

	bool De(T &c, bool check = true)
	{
		int ret;
		bool rtn = false;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
		    configASSERT(ret == pdPASS);
        }
        if(!check || l() > 0)
		{
			c = array[head];
			if(head == alen - 1)
			{
				head = 0;
			}
			else
			{
				head++;
			}
			rtn = true;
		}
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
        return rtn;
	}
	bool DeBlock(T *data, int len, int overlap, bool check = true)
	{
		int ret;
		bool rtn = false;
        if(threadSafe)
        {
            ret = xSemaphoreTake(mtx, portMAX_DELAY);
		    configASSERT(ret == pdPASS);
        }
        if(!check || l() >= len)
		{
        	if(head + len <= alen)
        	{
            	memcpy((void *)&(data[0]), (void *)&(array[head]), sizeof(T)*len);
        	}
        	else
        	{
        		int l0 = alen - head;
        		int l1 = len - l0;
            	memcpy((void *)&(data[0]), (void *)&(array[head]), sizeof(T)*l0);
            	memcpy((void *)&(data[l0]), (void *)&(array[0]), sizeof(T)*l1);
        	}
        	len = len - overlap;
        	len = len > 0 ? len : 0;
        	if(head + len <= alen - 1)
        		head += len;
        	else
        		head = head + len - alen;
	        rtn = true;
		}
        if(threadSafe)
        {
            ret = xSemaphoreGive(mtx);
            configASSERT(ret == pdPASS);
        }
        return rtn;
	}
};

#endif
