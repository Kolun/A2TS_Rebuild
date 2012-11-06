#include "include/parser.h"

int commandCheck( std::wstring messageWS )
{
   QString message;
   //общее регулярное выражение для команд
   QRegExp commandRegExp("\[A2TS_CMD\][a-zA-Z]{3}\[\/A2TS_CMD\](\[A2TS_ARG\](([0-9]+|[0-9]+\.[0-9]+|[a-zA-Z0-9]+);)*\[\/A2TS_ARG\])?");
   message.fromUtf16( (ushort*)messageWS.c_str() );
   if( !commandRegExp.exactMatch( message ) )
   {
     return 1;
   }
   QString command = message.mid( I_COMMAND_POSITION, I_COMMAND_LENGTH );
   command.toUpper();
   if( command == "POS" )
   {
     int current_end,
         current_begin = 34;
     bool notError = true;
     argsComPOS tmp_args;

     //парсим аргументы
     for( int i = 0; ( i < 35 ) && notError; ++i )
     {
       current_end = message.indexOf( ";", current_begin );
       if ( current_end != -1 )
       {
         QString arg = message.mid( current_begin, current_end - current_begin );
         current_begin = current_end + 1;
         switch( i )
         {

           case 0:
             tmp_args.posX = arg.toInt( &notError, 10 );
             //действие для posX
             break;

           case 1:
             tmp_args.posY = arg.toInt( &notError, 10 );
             //действие для posY
             break;

           case 2:
             tmp_args.posZ = arg.toInt( &notError, 10 );
             //действие для posZ
             break;

           case 3:
             tmp_args.Dir = arg.toInt( &notError, 10 );
             //действие для Dir
             break;

           case 4:
             tmp_args.vehId = arg;
             //действие для vehId
             break;

           case 5:
             tmp_args.isOut = arg.toInt( &notError, 10 );
             //действие для isOut
             break;

             //#################=KV=0=#####################
           case 6:
             tmp_args.kvChan0 = arg.toFloat( &notError );
             //действие для kvChan0
             break;

           case 7:
             tmp_args.kvVol0 = arg.toFloat( &notError );
             //действие для kvVol0
             break;

           case 8:
             tmp_args.kvPos0 = arg.toInt( &notError, 10 );
             //действие для kvPos0
             break;

             //#################=KV=1=#####################
           case 9:
             tmp_args.kvChan1 = arg.toFloat( &notError );
             //действие для kvChan1
             break;

           case 10:
             tmp_args.kvVol1 = arg.toFloat( &notError );
             //действие для kvVol1
             break;

           case 11:
             tmp_args.kvPos1 = arg.toInt( &notError, 10 );
             //действие для kvPos1
             break;

             //#################=KV=2=#####################
           case 12:
             tmp_args.kvChan2 = arg.toFloat( &notError );
             //действие для kvChan2
             break;

           case 13:
             tmp_args.kvVol2 = arg.toFloat( &notError );
             //действие для kvVol2
             break;

           case 14:
             tmp_args.kvPos2 = arg.toInt( &notError, 10 );
             //действие для kvPos2
             break;

             //#################=KV=3=#####################
           case 15:
             tmp_args.kvChan3 = arg.toFloat( &notError );
             //действие для kvChan3
             break;

           case 16:
             tmp_args.kvVol3 = arg.toFloat( &notError );
             //действие для kvVol3
             break;

           case 17:
             tmp_args.kvPos3 = arg.toInt( &notError, 10 );
             //действие для kvPos3
             break;

           case 18:
             tmp_args.kvActive = arg.toInt( &notError, 10 );
             //действие для kvActive
             break;

           case 19:
             tmp_args.kvSide = arg.toInt( &notError, 10 );
             //действие для kvSide
             break;

             //#################=DV=0=#####################
           case 20:
             tmp_args.dvChan0 = arg.toFloat( &notError );
             //действие для dvChan0
             break;

           case 21:
             tmp_args.dvVol0 = arg.toFloat( &notError );
             //действие для dvVol0
             break;

           case 22:
             tmp_args.dvPos0 = arg.toInt( &notError, 10 );
             //действие для dvPos0
             break;

             //#################=DV=1=#####################
           case 23:
             tmp_args.dvChan1 = arg.toFloat( &notError );
             //действие для dvChan1
             break;

           case 24:
             tmp_args.dvVol1 = arg.toFloat( &notError );
             //действие для dvVol1
             break;

           case 25:
             tmp_args.dvPos1 = arg.toInt( &notError, 10 );
             //действие для dvPos1
             break;

             //#################=DV=2=#####################
           case 26:
             tmp_args.dvChan2 = arg.toFloat( &notError );
             //действие для dvChan2
             break;

           case 27:
             tmp_args.dvVol2 = arg.toFloat( &notError );
             //действие для dvVol2
             break;

           case 28:
             tmp_args.dvPos2 = arg.toInt( &notError, 10 );
             //действие для dvPos2
             break;

             //#################=DV=3=#####################
           case 29:
             tmp_args.dvChan3 = arg.toFloat( &notError );
             //действие для dvChan3
             break;

           case 30:
             tmp_args.dvVol3 = arg.toFloat( &notError );
             //действие для dvVol3
             break;

           case 31:
             tmp_args.dvPos3 = arg.toInt( &notError, 10 );
             //действие для dvPos3
             break;

           case 32:
             tmp_args.dvActive = arg.toInt( &notError, 10 );
             //действие для dvActive
             break;

           case 33:
             tmp_args.dvSide = arg.toInt( &notError, 10 );
             //действие для dvSide
             break;

           case 34:
             tmp_args.TAN = arg.toInt( &notError, 10 );
             //действие для TAN
             break;

         } //конец switch
       }
     } //конец for для аргументов
     if( !notError ) return 2; //ошибка преобразования аргумента
   } //конец обработки команды POS
}
