﻿#ifndef PARSER_H
#define PARSER_H

#include <qregexp>
#include <qstring>

#pragma pack(push, 1)

struct argsComPOS
{
  int posX;         //позиция игрока по горизонтали.
  int posY;         //позиция игрока по вертикали.
  int posZ;         //высота по отношению к морю.
  int Dir;          //направление взгляда игрока.
  QString vehId;  	//идентификатор техники (если 0 - на ногах).
  int isOut;      	//индикатор высунности (не используется если vehId = 0)
  float kvChan0;  	//частота первого канала КВ.
  float kvVol0;   	//громкость первого канал КВ.
  int kvPos0;     	//в каком ухе слышно первый канал КВ.
  float kvChan1;  	//частота второго канал КВ.
  float kvVol1;   	//громкость второго канала КВ.
  int kvPos1;     	//в каком ухе слышно второй канал КВ.
  float kvChan2;  	//частота третьего канала КВ.
  float kvVol2;   	//громкость третьего канала КВ.
  int kvPos2;     	//в каком ухе слышно третий канал КВ.
  float kvChan3;  	//частота четвёртого канала КВ.
  float kvVol3;   	//громкость четвёртого канала КВ.
  int kvPos3;     	//в каком ухе слышно четвёртый канал КВ.
  int kvActive;     //указание какой канал КВ активен (в какой мы говорим).
  int kvSide;     	//указание какой стороне принадлежит данная КВ рация.
  float dvChan0;  	//частота первого канала ДВ.
  float dvVol0;   	//громкость первого канал ДВ.
  int dvPos0;     	//в каком ухе слышно первый канал ДВ.
  float dvChan1;  	//частота второго канал ДВ.
  float dvVol1;   	//громкость второго канала ДВ.
  int dvPos1;     	//в каком ухе слышно второй канал ДВ.
  float dvChan2;  	//частота третьего канала ДВ.
  float dvVol2;   	//громкость третьего канала ДВ.
  int dvPos2;     	//в каком ухе слышно третий канал ДВ.
  float dvChan3;  	//частота четвёртого канала ДВ.
  float dvVol3;   	//громкость четвёртого канала ДВ.
  int dvPos3;     	//в каком ухе слышно четвёртый канал КВ.
  int dvActive;     //указание какой канал КВ активен (в какой мы говорим).
  int dvSide;     	//указание какой стороне принадлежит данная КВ рация.
  int TAN;          //Тангента (какую кнопку нажал пользователь) (0 - Голос, 1 - КВ, 2 - ДВ)
};
#pragma pack(pop)

#define I_COMMAND_POSITION      10      //первый символ команды
#define I_COMMAND_LENGTH        3       //длина команды

//! Парсер
int commandCheck( std::wstring messageWS );

#endif // PARSER_H
