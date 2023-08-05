
/*Программа выполняет умножение двух дробных чисел с плавающей запятой, путём форматирования чисел в формат IEEE 754 и побитовых операций над ними.
Имеется возможность уменьшения длинны мантиссы. */




#include <iostream>
#include <bitset>
#include <climits>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;


// Функция разбивает число на отдельные битсеты: знак, экспоненту, мантиссу.
void mod_bin_float(bitset<32> &BIN_FLOAT, bitset<24> &MANTISA, bitset<8> &EXHIBITOR, bitset<1> &SIGN, int MAN_LENGTH)
{
    MANTISA[23] = 1;
    for (int i = 22; i >= 24 - MAN_LENGTH; i--)
    {
        MANTISA[i] = BIN_FLOAT[i];
    }

    for (int i = 23; i <= 30; i++)
    {
        EXHIBITOR[i - 23] = BIN_FLOAT[i];
    }

    SIGN[0] = BIN_FLOAT[31];

    BIN_FLOAT[31] = SIGN[0];

    for (int i = 30; i >= 23; i--)
    {
        BIN_FLOAT[i] = EXHIBITOR[i - 23];
    }
    for (int i = 22; i >= 0; i--)
    {
        BIN_FLOAT[i] = MANTISA[i];
    }
}

// Функция умножает и округляет мантиссы.
void mantisa_multiply(bitset<24>& MANTISA_RESULT, bitset<24>& MANTISA_1, bitset<24>& MANTISA_2, bool &EXP_ADD, int MAN_LENGTH)
{
    bool flag = 1;
    bool rounding = 0;
    bitset<48> buf(0);

    for (int i = 0; i < 24; i++)
    {
        if (MANTISA_2[i] != 0)
        {
            for (int j = i; j < i + 24; j++)
            {
               if (MANTISA_2[i] and MANTISA_1[j - i])
               {
                   if (buf[j] and MANTISA_1[j - i])
                   {
                       for (int k = j; flag; k++)
                       {
                           if (k == j)
                           {
                               continue;
                           }
                           else if (buf[k] == 0)
                           {
                               buf[k] = 1;
                               flag = false;
                           }
                           else if (buf[k] == 1)
                           {
                               buf[k] = 0;
                           }
                       }
                   }
                   flag = true;
                   buf[j] = buf[j] xor MANTISA_1[j - i];
               }
            }
            
        }
        
    }


    cout << "mant A x mant B: \n";
    cout << MANTISA_1[23] << ".";
    for (int i = 22; i >= 0; i--) 
    {
        cout << MANTISA_1[i];
    }

    cout << " x ";

    cout << MANTISA_2[23] << ".";
    for (int i = 22; i >= 0; i--)
    {
        cout << MANTISA_2[i];
    }

    cout << " = ";

    cout << buf[47] << buf[46] << ".";
    for (int i = 45; i >= 0; i--)
    {
        cout << buf[i];
    }
    cout << endl;

    if (!buf[47])
    {
        buf <<= 1;
        EXP_ADD = false;
    }
    else
    {
        EXP_ADD = true;
    }

    bool buf_of_round = 0;
    for (int i = (47 - (MAN_LENGTH + 1)); i >= 47 - (MAN_LENGTH * 2 - 1); i--)
    {
        if (buf[i])
        {
            buf_of_round = 1;
        }
    }

    if ((buf[(47 - MAN_LENGTH) + 1] and buf[47 - MAN_LENGTH]) or (buf[47 - MAN_LENGTH] and (buf_of_round)))
    {
        rounding = 1;
        for (int i = (47 - MAN_LENGTH) + 1; flag; i++)
        {
            if (buf[i] == 0)
            {
                buf[i] = 1;
                flag = false;
            }
            else if (buf[i] != 0)
            {
                buf[i] = 0;
            }
        }
        flag = true;
    }

    buf >>= 24;
    for (int i = 23; i >= (23 - MAN_LENGTH) + 1; i--)
    {
        MANTISA_RESULT[i] = buf[i];
    }

    cout << "mant result rounded: " << MANTISA_RESULT[23] << ".";
    for (int i = 22; i >= 0; i--)
    {
        cout << MANTISA_RESULT[i];
    }

    cout.setf(ios::boolalpha);
    cout << "\trounding: " << rounding;
    cout.unsetf(ios::boolalpha);

    cout << endl << endl;
}

// Функция определяет знак результата.
void sign_multiply(bitset<1> &SIGN_RESULT, bitset<1> &SIGN_1, bitset<1> &SIGN_2)
{
    SIGN_RESULT = SIGN_1 ^= SIGN_2;
}

// Функция суммирует экспоненты.
void exhibitor_adder(bitset<8> &EXHIBITOR_RESULT, bitset<8> &EXHIBITOR_1, bitset<8> &EXHIBITOR_2, bitset<24> MANTISA_RESULT, bool &EXP_ADD)
{
    unsigned long int exhibitor_dec_1,exhibitor_dec_2, exhibitor_dec_result;
    exhibitor_dec_1 = EXHIBITOR_1.to_ulong();
    exhibitor_dec_2 = EXHIBITOR_2.to_ulong();
    exhibitor_dec_result = (exhibitor_dec_1 - 127) + (exhibitor_dec_2 - 127);

    if (EXP_ADD)
    {
        exhibitor_dec_result++;
    }

    bitset<8> exhibitor_bin_result(exhibitor_dec_result + 127);
    EXHIBITOR_RESULT = exhibitor_bin_result;

}

// Функция соединяет результаты: знак, экспоненту, мантиссу.
void linker(bitset<32>& NUM_RESULT_BIN_FLOAT, bitset<1>& SIGN_RESULT, bitset<8>& EXHIBITOR_RESULT, bitset<23>& MANTISA_RESULT_FOR_VIEW)
{
    NUM_RESULT_BIN_FLOAT[31] = SIGN_RESULT[0];

    for (int i = 30; i >= 23; i--)
    {
        NUM_RESULT_BIN_FLOAT[i] = EXHIBITOR_RESULT[i - 23];
    }
    for (int i = 22; i >= 0; i--)
    {
        NUM_RESULT_BIN_FLOAT[i] = MANTISA_RESULT_FOR_VIEW[i];
    }

    union
    {
        unsigned long int input;
        float output;
    } data;
    data.input = NUM_RESULT_BIN_FLOAT.to_ulong();

    cout << "result: hex: " << hex << NUM_RESULT_BIN_FLOAT.to_ulong() << "\tbin: " << SIGN_RESULT << " " << EXHIBITOR_RESULT << " " << MANTISA_RESULT_FOR_VIEW << dec;
    cout << setprecision(19) << "\tdec: " << data.output;
    cout << endl << endl;
}

// Вспомогательная функция, которая добавляет точку между мнимой единицей и началом мантиссы для вывода в консоль.
void add_point(bitset<23>& MANTISA_FOR_VIEW, bitset<24>& MANTISA_NOT_FOR_VIEW, int MAN_LENGTH)
{
    for (int i = 22; i >= 24 - MAN_LENGTH; i--)
    {
        MANTISA_FOR_VIEW[i] = MANTISA_NOT_FOR_VIEW[i];
    }
}

int main(int argc, char * argv[])
{
    setlocale(LC_ALL, "Russian");

    int exp_length = 8;
    int man_length = 24;
    
    bitset<24> mantisa_1(0);
    bitset<24> mantisa_2(0);
    bitset<23> mantisa_1_for_view(0);
    bitset<23> mantisa_2_for_view(0);
    bitset<8> exhibitor_1(0);
    bitset<8> exhibitor_2(0);
    bitset<8> exhibitor_result(0);
    bitset<1> sign_1(0);
    bitset<1> sign_2(0);
    bitset<1> sign_result(0);
    bitset<24> mantisa_result(0);
    bitset<23> mantisa_result_for_view(0);
    bitset<32> num_result_bin_float(0);
    float num_1_dec;
    float num_2_dec;
    string value_1;
    string value_2;
    bool exp_add = false;
    char type_1;
    char type_2;
    
    cout << "Ввод размера мантиссы (1 - 24)\n";
    cin >> man_length;
    if (man_length >= 1 and man_length <= 24)
    {
        cout << "Ввод операндов A и B в формате: тип(h/b/d) число тип(h/b/d) число ( h - HEX, b - BIN, d - DEC (float) )" << endl;

        cin >> type_1;
        switch (type_1)
        {
        case 'h':
        {
            cin >> value_1;
            int num = stoi(value_1, 0, 16);

            union
            {
                int input;
                float   output;
            } data;

            data.input = num;
            num_1_dec = data.output;
            break;
        }
        case 'b':
        {
            cin >> value_1;
            int num = stoi(value_1, 0, 2);

            union
            {
                int input;
                float   output;
            } data;

            data.input = num;
            num_1_dec = data.output;
            break;
        }
        case 'd':
            cin >> num_1_dec;
            break;
        }


        cin >> type_2;
        switch (type_2)
        {
        case 'h':
        {
            cin >> value_2;
            int num = stoi(value_2, 0, 16);

            union
            {
                int input;
                float   output;
            } data;

            data.input = num;
            num_2_dec = data.output;
            break;
        }
        case 'b':
        {
            cin >> value_2;
            int num = stoi(value_2, 0, 2);

            union
            {
                int input;
                float   output;
            } data;

            data.input = num;
            num_2_dec = data.output;
            break;
        }
        case 'd':
            cin >> num_2_dec;
            break;
        }

        cout << endl;

        if ((value_1 == "nan") or (value_2 == "nan"))
        {
            exhibitor_result.set();
            mantisa_result.set(0);
            cout << "Результат: NaN" << endl;
            linker(num_result_bin_float, sign_result, exhibitor_result, mantisa_result_for_view);
        }
        else if ((value_1 == "inf") or (value_2 == "inf"))
        {
            exhibitor_result.set();
            cout << "Результат: inf" << endl;
            linker(num_result_bin_float, sign_result, exhibitor_result, mantisa_result_for_view);
        }
        else if ((value_1 == "0") or (value_2 == "0"))
        {
            cout << "Результат: 0" << endl;
            linker(num_result_bin_float, sign_result, exhibitor_result, mantisa_result_for_view);
        }
        else if ((num_1_dec == 0) or (num_2_dec == 0))
        {
            cout << "Введены неправильные значения.";
        }
        else
        {
            union
            {
                float input;
                int   output;
            } data1;

            data1.input = num_1_dec;
            bitset<32> num_1_bin_float(data1.output);

            mod_bin_float(num_1_bin_float, mantisa_1, exhibitor_1, sign_1, man_length);

            union
            {
                unsigned long int input;
                float output;
            } data2;

            data2.input = num_1_bin_float.to_ulong();
            num_1_dec = data2.output;
            data1.input = num_1_dec;

            add_point(mantisa_1_for_view, mantisa_1, man_length);

            string str = mantisa_1_for_view.to_string();
            cout << "A: hex: " << hex << data1.output << "\tbin: " << sign_1 << " " << exhibitor_1 << " " << str.substr(0, man_length - 1) << dec;
            cout << setprecision(19) << "\tdec: " << num_1_dec;

            cout << endl;

            data1.input = num_2_dec;
            bitset<32> num_2_bin_float(data1.output);
            mod_bin_float(num_2_bin_float, mantisa_2, exhibitor_2, sign_2, man_length);


            data2.input = num_2_bin_float.to_ulong();
            num_2_dec = data2.output;
            data1.input = num_2_dec;

            add_point(mantisa_2_for_view, mantisa_2, man_length);

            str = mantisa_2_for_view.to_string();
            cout << "B: hex: " << hex << data1.output << "\tbin: " << sign_2 << " " << exhibitor_2 << " " << str.substr(0, man_length - 1) << dec;
            cout << setprecision(19) << "\tdec: " << num_2_dec;

            cout << endl << endl;

            mantisa_multiply(mantisa_result, mantisa_1, mantisa_2, exp_add, man_length);

            add_point(mantisa_result_for_view, mantisa_result, man_length);

            sign_multiply(sign_result, sign_1, sign_2);

            exhibitor_adder(exhibitor_result, exhibitor_1, exhibitor_2, mantisa_result, exp_add);

            linker(num_result_bin_float, sign_result, exhibitor_result, mantisa_result_for_view);

        }

    }
    else
    {
        cout << "Введено неправильное значение\n";
    }

    
    system("pause");
    return 0;
}
