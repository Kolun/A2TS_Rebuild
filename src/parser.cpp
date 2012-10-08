#include <QtCore/QRegExp>
#include <QtCore/QString>

#pragma pack(push, 1)
struct argsComPOS
{
  int posX;         //������� ������ �� �����������.
  int posY;         //������� ������ �� ���������.
  int posZ;         //������ �� ��������� � ����.
  int Dir;          //����������� ������� ������.
  QString vehId;  	//������������� ������� (���� 0 - �� �����).
  int isOut;      	//��������� ���������� (�� ������������ ���� vehId = 0)
  float kvChan0;  	//������� ������� ������ ��.
  float kvVol0;   	//��������� ������� ����� ��.
  int kvPos0;     	//� ����� ��� ������ ������ ����� ��.
  float kvChan1;  	//������� ������� ����� ��.
  float kvVol1;   	//��������� ������� ������ ��.
  int kvPos1;     	//� ����� ��� ������ ������ ����� ��.
  float kvChan2;  	//������� �������� ������ ��.
  float kvVol2;   	//��������� �������� ������ ��.
  int kvPos2;     	//� ����� ��� ������ ������ ����� ��.
  float kvChan3;  	//������� ��������� ������ ��.
  float kvVol3;   	//��������� ��������� ������ ��.
  int kvPos3;     	//� ����� ��� ������ �������� ����� ��.
  int kvActive;     //�������� ����� ����� �� ������� (� ����� �� �������).
  int kvSide;     	//�������� ����� ������� ����������� ������ �� �����.
  float dvChan0;  	//������� ������� ������ ��.
  float dvVol0;   	//��������� ������� ����� ��.
  int dvPos0;     	//� ����� ��� ������ ������ ����� ��.
  float dvChan1;  	//������� ������� ����� ��.
  float dvVol1;   	//��������� ������� ������ ��.
  int dvPos1;     	//� ����� ��� ������ ������ ����� ��.
  float dvChan2;  	//������� �������� ������ ��.
  float dvVol2;   	//��������� �������� ������ ��.
  int dvPos2;     	//� ����� ��� ������ ������ ����� ��.
  float dvChan3;  	//������� ��������� ������ ��.
  float dvVol3;   	//��������� ��������� ������ ��.
  int dvPos3;     	//� ����� ��� ������ �������� ����� ��.
  int dvActive;     //�������� ����� ����� �� ������� (� ����� �� �������).
  int dvSide;     	//�������� ����� ������� ����������� ������ �� �����.
  int TAN;          //�������� (����� ������ ����� ������������) (0 - �����, 1 - ��, 2 - ��)
};
#pragma pack(pop)

#define I_COMMAND_POSITION      10      //������ ������ �������
#define I_COMMAND_LENGTH        3       //����� �������

int commandCheck( std::wstring messageWS )
{
   QString message;
   //����� ���������� ��������� ��� ������
   QRegExp commandRegExp("\[A2TS_CMD\][a-zA-Z]{3}\[\/A2TS_CMD\](\[A2TS_ARG\](([0-9]+|[0-9]+\.[0-9]+|[a-zA-Z0-9]+);)*\[\/A2TS_ARG\])?");
   message.setUtf16( messageWS.c_str() );
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

     //������ ���������
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
             tmp_args.posX = arg.toInt( &error, 10 );
             //�������� ��� posX
             break;

           case 1:
             tmp_args.posY = arg.toInt( &error, 10 );
             //�������� ��� posY
             break;

           case 2:
             tmp_args.posZ = arg.toInt( &error, 10 );
             //�������� ��� posZ
             break;

           case 3:
             tmp_args.Dir = arg.toInt( &error, 10 );
             //�������� ��� Dir
             break;

           case 4:
             tmp_args.vehId = arg;
             //�������� ��� vehId
             break;

           case 5:
             tmp_args.isOut = arg.toInt( &error, 10 );
             //�������� ��� isOut
             break;

             //#################=KV=0=#####################
           case 6:
             tmp_args.kvChan0 = arg.toFloat( &error );
             //�������� ��� kvChan0
             break;

           case 7:
             tmp_args.kvVol0 = arg.toFloat( &error );
             //�������� ��� kvVol0
             break;

           case 8:
             tmp_args.kvPos0 = arg.toInt( &error, 10 );
             //�������� ��� kvPos0
             break;

             //#################=KV=1=#####################
           case 9:
             tmp_args.kvChan1 = arg.toFloat( &error );
             //�������� ��� kvChan1
             break;

           case 10:
             tmp_args.kvVol1 = arg.toFloat( &error );
             //�������� ��� kvVol1
             break;

           case 11:
             tmp_args.kvPos1 = arg.toInt( &error, 10 );
             //�������� ��� kvPos1
             break;

             //#################=KV=2=#####################
           case 12:
             tmp_args.kvChan2 = arg.toFloat( &error );
             //�������� ��� kvChan2
             break;

           case 13:
             tmp_args.kvVol2 = arg.toFloat( &error );
             //�������� ��� kvVol2
             break;

           case 14:
             tmp_args.kvPos2 = arg.toInt( &error, 10 );
             //�������� ��� kvPos2
             break;

             //#################=KV=3=#####################
           case 15:
             tmp_args.kvChan3 = arg.toFloat( &error );
             //�������� ��� kvChan3
             break;

           case 16:
             tmp_args.kvVol3 = arg.toFloat( &error );
             //�������� ��� kvVol3
             break;

           case 17:
             tmp_args.kvPos3 = arg.toInt( &error, 10 );
             //�������� ��� kvPos3
             break;

           case 18:
             tmp_args.kvActive = arg.toInt( &error, 10 );
             //�������� ��� kvActive
             break;

           case 19:
             tmp_args.kvSide = arg.toInt( &error, 10 );
             //�������� ��� kvSide
             break;

             //#################=DV=0=#####################
           case 20:
             tmp_args.dvChan0 = arg.toFloat( &error );
             //�������� ��� dvChan0
             break;

           case 21:
             tmp_args.dvVol0 = arg.toFloat( &error );
             //�������� ��� dvVol0
             break;

           case 22:
             tmp_args.dvPos0 = arg.toInt( &error, 10 );
             //�������� ��� dvPos0
             break;

             //#################=DV=1=#####################
           case 23:
             tmp_args.dvChan1 = arg.toFloat( &error );
             //�������� ��� dvChan1
             break;

           case 24:
             tmp_args.dvVol1 = arg.toFloat( &error );
             //�������� ��� dvVol1
             break;

           case 25:
             tmp_args.dvPos1 = arg.toInt( &error, 10 );
             //�������� ��� dvPos1
             break;

             //#################=DV=2=#####################
           case 26:
             tmp_args.dvChan2 = arg.toFloat( &error );
             //�������� ��� dvChan2
             break;

           case 27:
             tmp_args.dvVol2 = arg.toFloat( &error );
             //�������� ��� dvVol2
             break;

           case 28:
             tmp_args.dvPos2 = arg.toInt( &error, 10 );
             //�������� ��� dvPos2
             break;

             //#################=DV=3=#####################
           case 29:
             tmp_args.dvChan3 = arg.toFloat( &error );
             //�������� ��� dvChan3
             break;

           case 30:
             tmp_args.dvVol3 = arg.toFloat( &error );
             //�������� ��� dvVol3
             break;

           case 31:
             tmp_args.dvPos3 = arg.toInt( &error, 10 );
             //�������� ��� dvPos3
             break;

           case 32:
             tmp_args.dvActive = arg.toInt( &error, 10 );
             //�������� ��� dvActive
             break;

           case 33:
             tmp_args.dvSide = arg.toInt( &error, 10 );
             //�������� ��� dvSide
             break;

           case 34:
             tmp_args.TAN = arg.toInt( &error, 10 );
             //�������� ��� TAN
             break;

         } //����� switch
       }
     } //����� for ��� ����������
     if( !notError ) return 2; //������ �������������� ���������
   } //����� ��������� ������� POS
}
