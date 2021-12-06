#ifndef __KEY_H__
#define __KEY_H__

template <int HoldTick = -1, int RepeatTick = -1>
class LoyKey
{
private:
	bool (*getKey)(void *getKeyArg);
	void *getKeyArg;
    void (*keyEvent)(void *keyEventArg, bool currStatus, bool lastStatus, unsigned int time);
    void *keyEventArg;
    bool lastStatus, currStatus;
    unsigned int timeCnt;
public:
	LoyKey(bool (*getKey)(void*), void *getKeyArg, void (*keyEvent)(void *, bool, bool, unsigned int), void *keyEventArg)
    {
		this->getKey = getKey;
		this->getKeyArg = getKeyArg;
		this->keyEvent = keyEvent;
		this->keyEventArg = keyEventArg;
		lastStatus = false;
		currStatus = false;
		timeCnt = (unsigned int)-1;
	}
	void Tick()
	{
		currStatus = getKey(getKeyArg);
		if(currStatus != lastStatus)
		{
			keyEvent(keyEventArg, currStatus, lastStatus, timeCnt);
			timeCnt = 0;
		}
		else if(currStatus)
		{
			if(timeCnt >= HoldTick)
			{
				if((timeCnt - HoldTick) % RepeatTick == 0)
					keyEvent(keyEventArg, currStatus, lastStatus, timeCnt);
			}
		}
		lastStatus = currStatus;
		timeCnt++;
	}
};

#endif
