/*
 Copyright © 2021  TokiNoBug
This file is part of SlopeCraft.

    SlopeCraft is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlopeCraft is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SlopeCraft.  If not, see <https://www.gnu.org/licenses/>.
    
    Contact with me:
    github:https://github.com/ToKiNoBug
    bilibili:https://space.bilibili.com/351429231
*/


#ifndef _NBTWriter_Cpp
#define _NBTWriter_Cpp
#include "NBTWriter.h"
#include <iostream>

using namespace NBT;

template <typename T>
void NBT::IE2BE(T &Val)
{
    if (sizeof(T)<=1)return;
    char *s=(char*)&Val;
    for(short i=0;i*2+1<(signed)sizeof(T);i++)
    std::swap(s[i],s[sizeof(T)-1-i]);
}

template <typename T>
T NBT::IE2BE(T *Val)
{
    if (sizeof(T)<=1)return *Val;

    T Res=*Val;
    char *s=(char*)&Res;
    for(short i=0;i*2+1<(signed)sizeof(T);i++)
    std::swap(s[i],s[sizeof(T)-1-i]);
    return Res;
}
bool NBT::isSysBE()
{
    short S=1;char *temp=(char*)&S;
    if(temp[0]==0&&temp[1]==1)
    return true;
    return false;
}

NBTWriter::NBTWriter(const char*path, bool stdout_output)
{
    Stdout_output = stdout_output;
    allowEmergencyFill=true;
    isBE=isSysBE();
    ByteCount=0;
    if (!Stdout_output) {
    	File=new std::fstream(path,std::ios::out|std::ios::binary);
    }
        char temp[3]={10,0,0};
        this->write(temp,3);ByteCount+=3;
    isOpen=true;
    for(top=0;top<TwinStackSize;top++)
    {
        CLA[top]=114;
        Size[top]=114514;
    }

    top=-1;

}

NBTWriter::NBTWriter()
{
    allowEmergencyFill=true;
    isBE=isSysBE();
    ByteCount=0;
    File=NULL;//new fstream(path,ios::out|ios::binary);
        //char temp[3]={10,0,0};
        //this->write(temp,3);ByteCount+=3;
    isOpen=false;
    for(top=0;top<TwinStackSize;top++)
    {
        CLA[top]=114;
        Size[top]=114514;
    }

    top=-1;

}

template<typename T>
void NBTWriter::write(T* data, size_t len) {
	if (Stdout_output) {
		std::fwrite(data, sizeof(T), len, stdout);		
	} else {
		File->write(reinterpret_cast<const char*>(data), sizeof(T)*len);
	}
}

void NBTWriter::open(const char*path)
{
    if(isOpen)
    {
        return;
    }
    File=new std::fstream(path,std::ios::out|std::ios::binary);
    char temp[3]={10,0,0};
    this->write(temp,3);ByteCount+=3;
    isOpen=true;
}


NBTWriter::~NBTWriter()
{
    if(isOpen)close();
    delete File;
    return;
}

unsigned long long NBTWriter::close()
{
    if(isOpen)
    {
        if(!isEmpty())emergencyFill();

    this->write(&idEnd,1);ByteCount+=1;
    if (!Stdout_output)
    	File->close();
    }
    return ByteCount;
}

bool NBTWriter::isEmpty()
{
    return (top==-1);
}

bool NBTWriter::isFull()
{
    return top>=TwinStackSize;
}

bool NBTWriter::isListFinished()
{
    return (Size[top]<=0);
}

char NBTWriter::readType()
{
    return CLA[top];
}

bool NBTWriter::isInCompound()
{
    return isEmpty()||(readType()==0);
}

bool NBTWriter::isInList()
{
    return !isInCompound();
}

bool NBTWriter::typeMatch(char typeId)
{
    return(readType()==typeId);
}

void NBTWriter::endList()
{
    if(isInList()&&isListFinished())
    {
        pop();
        elementWritten();
    }
    return;
}

void NBTWriter::pop()
{
    if(!isEmpty()){
    top--;
    }
    else
        ;
    return;
}

void NBTWriter::push(char typeId,int size)
{
    if(!isFull())
    {
        top++;
        CLA[top]=typeId;
        Size[top]=size;
        //qDebug()<<"push成功，栈顶CLA="<<(short)CLA[top]<<"，Size="<<Size[top];
    }
    else
    return;
}

void NBTWriter:: elementWritten()
{
    if(isInList()&&!isListFinished())
    Size[top]--;
    if(isListFinished())
    endList();
    return;
}

int NBTWriter::writeEnd()
{
    this->write(&idEnd,1);
    return 1;
}

template <typename T>
int NBTWriter::writeSingleTag(char typeId,const char*Name,T value)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    if(!isBE)
    {
        IE2BE(writeNameL);IE2BE(value);//value不需要读取，只需要写入
    }
    if (isInCompound())//写入为完整的Tag
    {
        //qDebug()<<"写入为文件夹内的id"<<(short)typeId;
        this->write(&typeId,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        this->write((char*)&value,sizeof(T));ThisCount+=sizeof(T);
    }

    if (isInList()&&typeMatch(typeId))//写入为列表中的tag
    {
        //qDebug()<<"写入为列表中的id"<<(short)typeId;
        this->write((char*)&value,sizeof(T));ThisCount+=sizeof(T);
        elementWritten();

    }
    ByteCount+=ThisCount;
    return ThisCount;
}



int NBTWriter::writeLongDirectly(const char*Name,long long value)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    if(!isBE)
    {
        IE2BE(writeNameL);//value不需要读取，只需要写入
    }
    if (isInCompound())//写入为完整的Tag
    {
        //qDebug()<<"写入为文件夹内的id"<<(short)NBT::idLong;
        this->write(&NBT::idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        this->write((char*)&value,sizeof(long long));ThisCount+=sizeof(long long);
    }

    if (isInList()&&typeMatch(NBT::idLong))//写入为列表中的tag
    {
        //qDebug()<<"写入为列表中的id"<<(short)NBT::idLong;
        this->write((char*)&value,sizeof(long long));ThisCount+=sizeof(long long);
        elementWritten();

    }
    ByteCount+=ThisCount;
    return ThisCount;
}


int NBTWriter::writeCompound(const char*Name)
{
    if (!isOpen)return 0;
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    if(!isBE)
    {
        IE2BE(writeNameL);
    }
    if(isInCompound())
    {
        this->write(&idCompound,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        push(idEnd,0);
        ByteCount+=ThisCount;
        return ThisCount;
    }

    if (isInList()&&typeMatch(idCompound))
    {
        //writeNothing
        push(idEnd,0);
        ByteCount+=ThisCount;
        return ThisCount;
    }

    ByteCount+=ThisCount;
    return ThisCount;

}

int NBTWriter::endCompound()
{
    if(!isOpen)return 0;
    int ThisCount=0;
    if(isInCompound())
    {
        ThisCount+=writeEnd();
        pop();//This pop means end a Compound
        elementWritten();
        ByteCount+=ThisCount;
        return ThisCount;
    }
    return ThisCount;
}

int NBTWriter::writeListHead(const char*Name,char TypeId,int listSize)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    int writeListSize=listSize;//listSize->readListSize
    if(!isBE){IE2BE(writeNameL);IE2BE(writeListSize);}

    if(isInCompound())
    {
        this->write(&idList,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        this->write(&TypeId,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeListSize,sizeof(int));ThisCount+=sizeof(int);
        push(TypeId,listSize);
        ByteCount+=ThisCount;
        if(listSize==0)elementWritten();
        return ThisCount;
    }

    if(isInList()&&typeMatch(idList))
    {
        this->write(&TypeId,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeListSize,sizeof(int));ThisCount+=sizeof(int);
        push(TypeId,listSize);
        ByteCount+=ThisCount;
        if(listSize==0)elementWritten();
        return ThisCount;
    }
    return ThisCount;

}

int NBTWriter::writeByte(const char*Name,char value)
{
    return writeSingleTag(idByte,Name,value);
}

int NBTWriter::writeShort(const char*Name,short value)
{
    return writeSingleTag(idShort,Name,value);
}

int NBTWriter::writeInt(const char*Name,int value)
{
    return writeSingleTag(idInt,Name,value);
}

int NBTWriter::writeLong(const char*Name,long long value)
{
    return writeSingleTag(idLong,Name,value);
}

int NBTWriter::writeFloat(const char*Name,float value)
{
    return writeSingleTag(idFloat,Name,value);
}

int NBTWriter::writeDouble(const char*Name,double value)
{
    return writeSingleTag(idDouble,Name,value);
}

int NBTWriter::writeLongArrayHead(const char*Name,int arraySize)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    int writeArraySize=arraySize;//arraSize->readArraySize
    if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}

    if(isInCompound())
    {
        this->write(&idLongArray,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        //this->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idLong,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }

    if(isInList()&&typeMatch(idLongArray))
    {
        //this->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idLong,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }
    return ThisCount;
}

int NBTWriter::writeByteArrayHead(const char*Name,int arraySize)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    int writeArraySize=arraySize;//arraSize->readArraySize
    if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}

    if(isInCompound())
    {
        this->write(&idByteArray,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        //this->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idByte,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }

    if(isInList()&&typeMatch(idByteArray))
    {
        //this->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idByte,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }
    return ThisCount;
}

int NBTWriter::writeIntArrayHead(const char*Name,int arraySize)
{
    int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
    int writeArraySize=arraySize;//arraySize->readArraySize
    if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}

    if(isInCompound())
    {
        this->write(&idIntArray,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;

        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idInt,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }

    if(isInList()&&typeMatch(idIntArray))
    {
        //this->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
        push(idInt,arraySize);
        ByteCount+=ThisCount;
        if(arraySize==0)elementWritten();
        return ThisCount;
    }
    return ThisCount;
}

int NBTWriter::writeString(const char*Name,const char*value)
{
    int ThisCount=0;
    short realNameL=strlen(Name),writeNameL=realNameL;
    short realValL=strlen(value),writeValL=realValL;
    if(!isBE){IE2BE(writeNameL);IE2BE(writeValL);}

    if(isInCompound())
    {
        this->write(&idString,sizeof(char));ThisCount+=sizeof(char);
        this->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
        this->write(Name,realNameL);ThisCount+=realNameL;
        this->write((char*)&writeValL,sizeof(short));ThisCount+=sizeof(short);
        this->write(value,realValL);ThisCount+=realValL;
        ByteCount+=ThisCount;
        elementWritten();
        return ThisCount;
    }

    if(isInList()&&typeMatch(idString))
    {
        this->write((char*)&writeValL,sizeof(short));ThisCount+=sizeof(short);
        this->write(value,realValL);ThisCount+=realValL;
        ByteCount+=ThisCount;
        elementWritten();
        return ThisCount;
    }
    return ThisCount;
}

char NBTWriter::CurrentType()
{
    return readType();
}

int NBTWriter::emergencyFill()
{
    if(!allowEmergencyFill)return 0;
    if(isEmpty())return 0;
    int ThisCount=0;
    while(!isEmpty())
    {
        if(isInCompound()){ThisCount+=endCompound();continue;}
        switch (readType())
        {
            case idEnd:
                continue;
            case idByte:
                ThisCount+=writeByte("autoByte",114);
                continue;
            case idShort:
                ThisCount+=writeShort("autoShort",514);
                continue;
            case idInt:
                ThisCount+=writeInt("autoInt",114514);
                continue;
            case idLong:
                ThisCount+=writeLong("autoLong",1919810);
                continue;
            case idFloat:
                ThisCount+=writeFloat("autoFloat",114.514f);
                continue;
            case idDouble:
                ThisCount+=writeDouble("autoDouble",1919810.114514);
                continue;
            case idByteArray:
                ThisCount+=writeByteArrayHead("autoByteArray",1);
                continue;
            case idString:
                ThisCount+=writeString("autoString","FuckYUUUUUUUUUUUUUUUUU!");
                continue;
            case idList:
                ThisCount+=writeListHead("autoList",NBT::idInt,1);
                continue;
            case idCompound:
                ThisCount+=writeCompound("autoCompound");
                continue;
            case idIntArray:
                ThisCount+=writeIntArrayHead("autoIntArray",1);
                continue;
            case idLongArray:
                ThisCount+=writeLongArrayHead("autoLongArray",1);
                continue;
            default:
                continue;
        }
    }
    ThisCount+=writeString("TokiNoBug'sWarning","There's sth wrong with ur NBTWriter, the file format is completed automatically instead of manually.");
    return ThisCount;
}
unsigned long long NBTWriter::getByteCount()
{
    return ByteCount;
}

#endif
