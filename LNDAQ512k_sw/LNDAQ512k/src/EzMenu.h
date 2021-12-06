/*
 * EzMenu.h
 *
 *  Created on: Jun 17, 2020
 *      Author: loywong
 */

#ifndef SRC_EZMENU_H_
#define SRC_EZMENU_H_

#include <cmath>
#include <type_traits>
#include <stdlib.h>
#include "oled12864/lq12864.h"

enum class EzMIType
{
    Undef,
    Label,
    List,
    Eidt,
    Num,
    SubMenu
};

enum class EzMIAction
{
    Nothing,
    NeedRerender,
    EnteringEdit,
    FinishedEdit,
    CanceledEdit,
    ValueChange,
    SubMenuEnter
};

template<typename CT = unsigned int, unsigned int SL = 16>
class EzMenu;

template<typename CT = unsigned int, unsigned int SL = 16>
class EzMenuItem
{
private:
    bool isEnable = true;
    bool editing = false;
    int cursorPos = 0;
    EzMIType type = EzMIType::Undef;
    const char *const *list = NULL;
    int listLen = 1;
    int currIdx = 0;
    char value[SL + 1];
    int editStartPos = 0;
    int editLen = 1;
    double maxValue, minValue;
    EzMenu<CT, SL> *subMenu = NULL;

public:
    bool CanFocus = true;
    CT FColor = 0x000000;
    CT BColor = 0xffffff;
    CT DisFColor = 0x404040;
    CT DisBColor = 0xc0c0c0;
    CT FocFColor = 0xffffff;
    CT FocBColor = 0x000000;
    unsigned int PosX = 0;
    unsigned int PosY = 0;
    unsigned int Width = 6;
    unsigned int Height = 8;
    char *Prefix = "\0";
    char *Postfix = "\0";
    EzMenuItem(const char *value)
    {
        this->type = EzMIType::Label;
        configASSERT(strlen(value) <= SL);
        strcpy(this->value, value);
    }
    EzMenuItem(char *const *list, int listLen, int currIdx)
    {
        this->type = EzMIType::List;
        this->list = list;
        this->listLen = listLen;
        this->currIdx = currIdx;
        for(int i = 0; i < listLen; i++)
            configASSERT(strlen(list[i]) <= SL);
        strcpy(this->value, list[currIdx]);
    }
    EzMenuItem(const char *value, int editStartPos, int editLen)
    {
        this->type = EzMIType::Eidt;
        this->editStartPos = editStartPos;
        this->editLen = editLen;
        this->cursorPos = editStartPos;
        configASSERT(strlen(value) <= SL);
        strcpy(this->value, value);
    }
    EzMenuItem(const char *value, double min, double max, int editLen)
    {
        this->type = EzMIType::Num;
        this->minValue = min < max ? min : max;
        this->maxValue = max > min ? max : min;
        this->editLen = editLen;
        this->cursorPos = this->editLen - 1;
        configASSERT(strlen(value) <= SL);
        strcpy(this->value, value);
    }
    EzMenuItem(const char *value, EzMenu<CT, SL> *subMenu)
    {
        this->type = EzMIType::SubMenu;
        this->subMenu = subMenu;
        configASSERT(strlen(value) <= SL);
        strcpy(this->value, value);
    }
    EzMIType Type()
    {
        return this->type;
    }
    void Enable()
    {
        this->isEnabled = true;
    }
    void Disable()
    {
        this->editing = false;
        this->isEnabled = false;
    }
    bool IsEnable()
    {
        return this->isEnable;
    }
    bool IsEditing()
    {
        return this->editing;
    }
    void SetValue(const char *val)
    {
        configASSERT(strlen(value) <= SL);
        strcpy(this->value, val);
    }
    void Render(LQ12864 *disp, int xOffs, int yOffs)
    {
        char e[2];
        int x = xOffs + this->PosX;
        int y = yOffs + this->PosY;
        CT efc = isEnable? (editing? this->FocFColor : this->FColor) : this->DisFColor;
        CT fc = isEnable? this->FColor : this->DisFColor;
        CT ebc = isEnable? (editing? this->FocBColor : this->BColor) : this->DisBColor;
        CT bc = isEnable? this->BColor : this->DisBColor;
        x = disp->BufTextSmall(this->Prefix, x, y, fc, bc);
        switch(this->type)
        {
        case EzMIType::Eidt:
        case EzMIType::Num:
            e[0] = value[cursorPos];
            e[1] = '\0';
            value[cursorPos] = '\0';
            x = disp->BufTextSmall(this->value, x, y, fc, bc);
            value[cursorPos] = e[0];
            x = disp->BufTextSmall(e, x, y, efc, ebc);
            x = disp->BufTextSmall(this->value + cursorPos + 1, x, y, fc, bc);
            break;
        case EzMIType::List:
        case EzMIType::SubMenu:
            x = disp->BufTextSmall(this->value, x, y, efc, ebc);
            break;
        default:
            x = disp->BufTextSmall(this->value, x, y, efc, ebc);
            break;
        }
        x = disp->BufTextSmall(this->Postfix, x, y, fc, bc);
    }
    double GetDouble()
    {
        return ::atof(this->value);
    }
    float GetFloat()
    {
        return ::atoff(this->value);
    }
    int GetInt()
    {
        return ::atoi(this->value);
    }
    char *GetText()
    {
        return this->value;
    }
    int CurrIdx()
    {
        return this->currIdx;
    }
    EzMenu<CT, SL> *SubMenu()
    {
        return this->subMenu;
    }

    EzMIAction editAnsKey(char key)
    {
        static char temp[SL + 1] = {'\0'};
        if(editing)
        {
            switch(key)
            {
            case '\r':  // enter
                editing = false;
                return EzMIAction::FinishedEdit;
            case 0x1b:  // return
                editing = false;
                strcpy(this->value, temp);
                return EzMIAction::CanceledEdit;
            case 0x12:  // left
                if(cursorPos > editStartPos)
                    cursorPos--;
                else
                    cursorPos = editStartPos + editLen - 1;
                return EzMIAction::NeedRerender;
            case 0x14:  // right
                if(cursorPos < editStartPos + editLen - 1)
                    cursorPos++;
                else
                    cursorPos = editStartPos;
                return EzMIAction::NeedRerender;
            case 0x11:  //  up
                if(value[cursorPos] < 'z')
                    value[cursorPos]++;
                else
                    value[cursorPos] = '0';
                return EzMIAction::ValueChange;
            case 0x13: //   down
                if(value[cursorPos] > '0')
                    value[cursorPos]--;
                else
                    value[cursorPos] = 'z';
                return EzMIAction::ValueChange;
            default:
                if(key >= '0' && key <= 'z')
                {
                    value[cursorPos] = key;
                    if(cursorPos < this->editLen - 1)
                        cursorPos++;
                    else
                        cursorPos = 0;
                    return EzMIAction::ValueChange;
                }
                else
                {
                    return EzMIAction::Nothing;
                }
            }
        }
        else if(key == '\r')
        {
            editing = true;
            strcpy(temp, this->value);
            return EzMIAction::EnteringEdit;
        }
        return EzMIAction::Nothing;
    }
    EzMIAction listAnsKey(char key)
    {
        static char temp[SL + 1] = {'\0'};
        if(editing)
        {
            switch(key)
            {
            case '\r':  // enter
                editing = false;
                return EzMIAction::FinishedEdit;
            case 0x1b:  // return
                editing = false;
                strcpy(this->value, temp);
                return EzMIAction::CanceledEdit;
            case 0x11:  //  up
                if(currIdx > 0)
                    currIdx--;
                else
                    currIdx = this->listLen - 1;
                strcpy(this->value, this->list[currIdx]);
                return EzMIAction::ValueChange;
            case 0x13: //   down
                if(currIdx < this->listLen - 1)
                    currIdx++;
                else
                    currIdx = 0;
                strcpy(this->value, this->list[currIdx]);
                return EzMIAction::ValueChange;
            default:
                return EzMIAction::Nothing;
            }
        }
        else if(key == '\r')
        {
            editing = true;
            strcpy(temp, this->value);
            return EzMIAction::EnteringEdit;
        }
        return EzMIAction::Nothing;
    }
    EzMIAction numAnsKey(char key)
    {
        static char temp[SL + 1];
        static char t[SL + 1];
        int p;
        if(editing)
        {
            switch(key)
            {
            case '\r':  // enter
                editing = false;
                if(atof(value) > maxValue || atof(value) < minValue)
                {
                    strcpy(this->value, temp);
                }
                return EzMIAction::FinishedEdit;
            case 0x1b:  // return
                editing = false;
                strcpy(this->value, temp);
                return EzMIAction::CanceledEdit;
            case 0x12:  // left
                do
                {
                    if(cursorPos > 0)
                        cursorPos--;
                    else
                        cursorPos = this->editLen - 1;
                } while(value[cursorPos] == '.');
                return EzMIAction::NeedRerender;
            case 0x14:  // right
                do
                {
                    if(cursorPos < this->editLen - 1)
                        cursorPos++;
                    else
                        cursorPos = 0;
                } while(value[cursorPos] == '.');
                return EzMIAction::NeedRerender;
            case 0x11:  //  up
                strcpy(t, value);
                for(p = cursorPos; p >= 0; p--)
                {
                    if(value[p] == '.')
                        continue;
                    else if(value[p] < '9')
                    {
                        value[p]++;
                        break;
                    }
                    else
                    {
                        value[p] = '0';
                        continue;
                    }
                }
                if(p == -1 || atof(value) > maxValue)
                {
                    strcpy(value, t);
                    return EzMIAction::Nothing;
                }
                else
                {
                    return EzMIAction::ValueChange;
                }
            case 0x13: //   down
                strcpy(t, value);
                for(p = cursorPos; p >= 0; p--)
                {
                    if(value[p] == '.')
                        continue;
                    else if(value[p] > '0')
                    {
                        value[p]--;
                        break;
                    }
                    else
                    {
                        value[p] = '9';
                        continue;
                    }
                }
                if(p == -1 || atof(value) < minValue)
                {
                    strcpy(value, t);
                    return EzMIAction::Nothing;
                }
                else
                {
                    return EzMIAction::ValueChange;
                }
            default:
                if((key >= '0' && key <= '9') || key == '.')
                {
                    value[cursorPos] = key;
                    if(cursorPos < this->editLen - 1)
                        cursorPos++;
                    else
                        cursorPos = 0;
                    return EzMIAction::NeedRerender;
                }
                else
                    return EzMIAction::Nothing;
            }
        }
        else if(key == '\r')
        {
            editing = true;
            strcpy(temp, this->value);
            return EzMIAction::EnteringEdit;
        }
        return EzMIAction::Nothing;
    }

    EzMIAction AnswerKey(char key)
    {
        if(!this->isEnable)
        {
            return EzMIAction::Nothing;
        }
        switch(type)
        {
        case EzMIType::Eidt:
            return editAnsKey(key);
        case EzMIType::List:
            return listAnsKey(key);
        case EzMIType::Num:
            return numAnsKey(key);
        case EzMIType::SubMenu:
            if(key == '\r')
                return EzMIAction::SubMenuEnter;
        default:
            return EzMIAction::Nothing;
        }
    }
};

template<typename CT, unsigned int SL>
class EzMenu
{
private:
    EzMenuItem<CT, SL> **items = NULL;
    int itemsCount = 0;
    int currIdx = 0;
    char name[SL + 1];
public:
    bool IsFocused = false;
    CT FocusFillColor = 0xffffff;
    CT FocusLineColor = 0x000000;
    CT UnfocFillColor = 0xc0c0c0;
    CT UnfocLineColor = 0x404040;
    unsigned int PosX = 0, PosY = 0;
    unsigned int Width = 6, Height = 8;
    EzMenu(char *name, EzMenuItem<CT, SL> **items, int count)
    {
        this->items = items;
        this->itemsCount = count;
        configASSERT(strlen(name) <= SL);
        strcpy(this->name, name);
    }

    EzMenuItem<CT, SL> *CurrItem()
    {
        return items[currIdx];
    }

    void Render(LQ12864 *disp)
    {
        CT fc = IsFocused ? FocusFillColor : UnfocFillColor;
        CT lc = IsFocused ? FocusLineColor : UnfocLineColor;
        disp->BufBar(PosX, PosY, PosX + Width - 1, PosY + Height - 1, lc, fc);
        disp->BufTextSmall(this->name, PosX + 2, PosY, lc, fc);
        for(int i = 0; i < itemsCount; i++)
        {
            this->items[i]->Render(disp, PosX, PosY);
            if(currIdx == i)
            {
                if(this->items[i]->IsEditing())
                    disp->BufTextSmall((char *)"*", items[i]->PosX + PosX - 6, items[i]->PosY + PosY, lc, fc);
                else
                    disp->BufTextSmall((char *)">", items[i]->PosX + PosX - 6, items[i]->PosY + PosY, lc, fc);
            }
        }
    }
    EzMIAction AnswerKey(char key)
    {
        if(!IsFocused)
            return EzMIAction::Nothing;
        EzMIAction act = items[currIdx]->AnswerKey(key);
        if(!items[currIdx]->IsEditing())
        {
            switch(key)
            {
            case 0x11:  // up
                do
                {
                    if(currIdx > 0) currIdx--;
                    else currIdx = itemsCount - 1;
                }
                while(!items[currIdx]->CanFocus);
                return EzMIAction::NeedRerender;
            case 0x13:  // down
                do
                {
                    if(currIdx < itemsCount - 1) currIdx++;
                    else currIdx = 0;
                }
                while(!items[currIdx]->CanFocus);
                return EzMIAction::NeedRerender;
            }
        }
        return act;
    }
};
//
//template<typename CT, unsigned int SL>
//class EzMenuCollection
//{
//private:
//    EzMenu<CT, SL> **menus = NULL;
//    int menusCount = 0;
//    int currIdx = 0;
//public:
//    EzMenuCollection(EzMenu<CT, SL> **menus, int count)
//    {
//        this->menus = menus;
//        this->itemsCount = count;
//        configASSERT(strlen(name) <= SL);
//        strcpy(this->name, name);
//    }
//
//    EzMenuItem<CT, SL> *GetCurrItem()
//    {
//        return items[currIdx];
//    }
//
//    void Render(LQ12864 *disp)
//    {
//        CT fc = IsFocused ? FocusFillColor : UnfocFillColor;
//        CT lc = IsFocused ? FocusLineColor : UnfocLineColor;
//        disp->BufBar(PosX, PosY, PosX + Width - 1, PosY + Height - 1, lc, fc);
//        disp->BufTextSmall(this->name, PosX + 1, PosY + 2, lc, fc);
//        for(int i = 0; i < itemsCount; i++)
//        {
//            this->items[i]->Render(disp, PosX, PosY);
//            if(currIdx == i)
//            {
//                if(this->items[i]->IsEditing())
//                    disp->BufTextSmall((char *)"*", items[i]->PosX + PosX - 6, items[i]->PosY + PosY, lc, fc);
//                else
//                    disp->BufTextSmall((char *)">", items[i]->PosX + PosX - 6, items[i]->PosY + PosY, lc, fc);
//            }
//        }
//    }
//    EzMIAction AnswerKey(char key)
//    {
//        if(!IsFocused)
//            return EzMIAction::Nothing;
//        EzMIAction act = items[currIdx]->AnswerKey(key);
//        if(!items[currIdx]->IsEditing())
//        {
//            switch(key)
//            {
//            case 0x11:  // up
//                do
//                {
//                    if(currIdx > 0) currIdx--;
//                    else currIdx = itemsCount - 1;
//                }
//                while(!items[currIdx]->CanFocus);
//                return EzMIAction::NeedRerender;
//            case 0x13:  // down
//                do
//                {
//                    if(currIdx < itemsCount - 1) currIdx++;
//                    else currIdx = 0;
//                }
//                while(!items[currIdx]->CanFocus);
//                return EzMIAction::NeedRerender;
//            }
//        }
//        return act;
//    }
//};

#endif /* SRC_EZMENU_H_ */
