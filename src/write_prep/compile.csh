#!/bin/csh

setenv ESMADIR /gpfsm/dnb02/asewnath/GEOSadas/

ifort -o write_prepbufr_example.x m_pbutil.f m_pbmin.f write_prepbufr_example.f90 -I. -L $ESMADIR/Linux/lib/ -lNCEP_bufr_r8i4 -I $ESMADIR/Linux/include/GMAO_mpeu -lGMAO_mpeu -extend-source 132

