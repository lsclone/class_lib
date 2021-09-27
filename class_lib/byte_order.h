/****************************************************************
///
///  Copyright (c) 2020-2021
///  All rights reserved.
///
///  @brief     host or network byte order conversion
///
///  @version   v1.0
///  @author    Owen.Li
///
///                   bytes of each type
///  ----------------------------------------------------
///             win32    win64   linux32   linux64
///  ----------------------------------------------------
///  char         1        1        1         1
///  ----------------------------------------------------
///  short        2        2        2         2
///  ----------------------------------------------------
///  int          4        4        4         4
///  ----------------------------------------------------
///  long         4        4        4         8
///  ----------------------------------------------------
///  long long    8        8        8         8
///  ----------------------------------------------------
///  float        4        4        4         4
///  ----------------------------------------------------
///  double       8        8        8         8
///  ----------------------------------------------------
///  pointer      4        8        4         8
///  ----------------------------------------------------
///
****************************************************************/

#pragma once
#include <string>

namespace {

    /*
    * @brief    check byte order, big or little endian.
    * @return   true if little endian, false if big endian.
    */
    bool check_endian() {
        union {
            int  i;
            char c;
        } u;
        u.i = 1;
        return (u.c != 0);
    }

    /*
    * @brief    swap values of two variable
    */
    template<typename Ty>
    void swap(Ty& a, Ty& b) {
        a ^= b;
        b ^= a;
        a ^= b;
    }

    /*
    * @brief    in-place byte swap (2 bytes)
    * @param    [in]a   pointer to 16 bits variable needs to spin.
    */
    void spin16(void* a) {
        union {
            char  i1[2];
            short i2;
        } tmp;

        memcpy(&tmp.i2, a, sizeof(short));
        swap(tmp.i1[0], tmp.i1[1]);
        memcpy(a, &tmp.i2, sizeof(short));
    }

    /*
    * @brief    in-place byte swap (4 bytes)
    * @param    [in]a   pointer to 32 bits variable needs to spin.
    */
    void spin32(void* a) {
        union {
            char i1[4];
            int  i4;
        } tmp;

        memcpy(&tmp.i4, a, sizeof(int));
        swap(tmp.i1[0], tmp.i1[3]);
        swap(tmp.i1[1], tmp.i1[2]);
        memcpy(a, &tmp.i4, sizeof(int));
    }

    /*
    * @brief    in-place byte swap (8 bytes)
    * @param    [in]a   pointer to 64 bits variable needs to spin.
    */
    void spin64(void* a) {
        union {
            char i1[8];
            long long i8;
        } tmp;

        memcpy(&tmp.i8, a, sizeof(long long));
        swap(tmp.i1[0], tmp.i1[7]);
        swap(tmp.i1[1], tmp.i1[6]);
        swap(tmp.i1[2], tmp.i1[5]);
        swap(tmp.i1[3], tmp.i1[4]);
        memcpy(a, &tmp.i8, sizeof(long long));
    }

}

/*
* @brief    host byte order convert to network order.
* @param    [in]data    variable need to convert byte order.
* @return   variable has converted byte order.
*/
template<typename Ty>
Ty hton(Ty data) {
    /*
    * for network byte order is big endian,
    * check big or litter endian firstly,
    * if litter endian, then spin variable.
    */
    if (check_endian()) {
        switch (sizeof(data)) {
            case 2:
                spin16(&data);
                break;
            case 4:
                spin32(&data);
                break;
            case 8:
                spin64(&data);
                break;
            default:
                break;
        }
    }
    return data;
};

/*
* @brief    network byte order convert to host order.
* @param    [in]data    variable need to convert byte order.
* @return   variable has converted byte order.
*/
template<typename Ty>
Ty ntoh(Ty data) {
    return hton<Ty>(data);
};

/*
* usage
* 
int main(int argc, char* argv[]) {
    union {
        int i4;
        float f4;
    } data;
    data.f4 = 1.29786e+02;
    std::cout << hex << data.i4 << std::endl;

    data.f4 = hton(data.f4);
    std::cout << hex << data.i4 << std::endl;

    data.f4 = ntoh(data.f4);
    std::cout << hex << data.i4 << std::endl;
    std::cout << data.f4 << std::endl;

    union {
        long long l8;
        double d8;
    } data2;
    data2.d8 = 2.7654e+02;
    std::cout << hex << data2.l8 << std::endl;

    data2.d8 = hton(data2.d8);
    std::cout << hex << data2.l8 << std::endl;

    data2.d8 = ntoh(data2.d8);
    std::cout << hex << data2.l8 << std::endl;
    std::cout << data2.d8 << std::endl;

    return 0;
}
*
*/
