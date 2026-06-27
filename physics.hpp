// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#include <stdio.h>
#include <math.h>

#define PRINT_Q(q, unit)                        \
if ((q) > 0) {                                  \
    printf("%s", #unit);                        \
    if ((q) > 1) printf("^%d", (q));            \
}

// kilogram, meter, second, radian, degree kelvin
template <int Mass, int Len, int Time, int Angle, int Temp>
class Quantity {
private:
    double val;
public:
    Quantity(double v = 0): val(v) {}
    double value() const { return val; }

    template<int e>
    Quantity<Mass*e,Len*e,Time*e,Angle*e,Temp*e> pow() const {
        return Quantity<Mass*e,Len*e,Time*e,Angle*e,Temp*e>(::pow(value(), e));
    }

    void disp() const {
        printf("%g ", val);
        PRINT_Q(Mass, kg);
        PRINT_Q(Len, m);
        PRINT_Q(Time, s);
        PRINT_Q(Angle, rad);
        PRINT_Q(Temp, K);
        if (Mass < 0 || Len < 0 || Time < 0 || Angle < 0 || Temp < 0) {
            printf("/");
            PRINT_Q(-Mass, kg);
            PRINT_Q(-Len, m);
            PRINT_Q(-Time, s);
            PRINT_Q(-Angle, rad);
            PRINT_Q(-Temp, K);
        }
        printf("\n");
    }
};

template<int M, int L, int T, int A, int K>
Quantity<M,L,T,A,K> operator+(Quantity<M,L,T,A,K> a,
                              Quantity<M,L,T,A,K> b) {
    return Quantity<M,L,T,A,K>(a.value() + b.value());
}

template<int M, int L, int T, int A, int K>
Quantity<M,L,T,A,K> operator*(Quantity<M,L,T,A,K> a, double b) {
    return Quantity<M,L,T,A,K>(a.value() * b);
}

template<int M, int L, int T, int A, int K>
Quantity<M,L,T,A,K> operator*(double a, Quantity<M,L,T,A,K> b) {
    return Quantity<M,L,T,A,K>(a * b.value());
}

template<int M1, int L1, int T1, int A1, int K1,
         int M2, int L2, int T2, int A2, int K2>
Quantity<M1+M2,L1+L2,T1+T2,A1+A2,K1+K2>
operator*(Quantity<M1,L1,T1,A1,K1> a,
          Quantity<M2,L2,T2,A2,K2> b) {
    return Quantity<M1+M2,L1+L2,T1+T2,A1+A2,K1+K2>(a.value() * b.value());
}

template<int M1, int L1, int T1, int A1, int K1,
         int M2, int L2, int T2, int A2, int K2>
Quantity<M1-M2,L1-L2,T1-T2,A1-A2,K1-K2>
operator/(Quantity<M1,L1,T1,A1,K1> a,
          Quantity<M2,L2,T2,A2,K2> b) {
    return Quantity<M1-M2,L1-L2,T1-T2,A1-A2,K1-K2>(a.value() / b.value());
}

// Mass, Length, Time, Angle, Temperature
typedef Quantity< 0, 1,-2, 0, 0> Acceleration;
typedef Quantity< 0, 0, 0, 1, 0> Angle;
typedef Quantity< 0, 0,-1, 1, 0> AngularVelocity;
typedef Quantity< 0, 2, 0, 0, 0> Area;
typedef Quantity< 1,-3, 0, 0, 0> Density;
typedef Quantity< 1, 2,-2, 0, 0> Energy;
typedef Quantity< 1, 1,-2, 0, 0> Force;
typedef Quantity< 0, 0,-1, 0, 0> Frequency;
typedef Quantity< 0, 1, 0, 0, 0> Length;
typedef Quantity< 1, 0, 0, 0, 0> Mass;
typedef Quantity< 1, 2,-3, 0, 0> Power;
typedef Quantity< 1,-1,-2, 0, 0> Pressure;
typedef Quantity< 0, 2,-2, 0,-1> SpecificHeat;
typedef Quantity< 0, 0, 0, 0, 1> kelvin;
typedef Quantity< 0, 0, 1, 0, 0> Time;
typedef Quantity< 0, 1,-1, 0, 0> Velocity;
typedef Quantity< 0, 3, 0, 0, 0> Volume;

const Mass      kg  = 1;
const Length    m   = 1;
const Time      s   = 1;
const Frequency hz  = 1;
const Angle     rad = 1;
const Force     N   = 1;
const double    Pi  = 3.141592653589793238462;

#define DEFINE_CONSTANT(name, value, M, L, T, A, K) \
const Quantity<M,L,T,A,K> name = Quantity<M,L,T,A,K>(value)

namespace constants {
    DEFINE_CONSTANT(GC,  6.6720e-11, -1, 3, -2, 0,  0); // Gravitational Constant
    DEFINE_CONSTANT(SBC, 5.6703e-8,   1, 0, -3, 0, -4); // Stefan-Boltzmann Constant
    DEFINE_CONSTANT(SLC, 299792458,   0, 1, -1, 0,  0); // Speed of light
    DEFINE_CONSTANT(SMC, 1.98855e30,  1, 0,  0, 0,  0); // Solar mass
}


inline double sin(Angle theta) { return sin(theta.value()); }
