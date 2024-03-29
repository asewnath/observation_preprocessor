 program write_prepbufr_example

  use m_pbutil, only: init_bufr, end_bufr
  use m_pbmin, only:  write_cat6_wind

  integer                       :: nobs, i, ndate
  real,dimension(:),allocatable :: pres, uwnd, vwnd, lat, lon, time, wind_error, pres_error, wind_qcflag, pres_qcflag, obs_type, sat_id

!  real                          :: miss = 10.0e10
  real                          :: miss=9999.9

  character*8  :: stationid = 'GMAO_AMV'   ! this can be really anything.  I like to use the data or processing source, perhaps (e.g. SSEC for Wisconsin or NRLSUPOB)
  character*8  :: subset    = 'SATWND'     ! this is important - if you look at the table below, this ties to that
  character*80 :: outfn     = './example.bufr' ! output file
  character*80 :: table_fn  = './qscat_tabl' ! this table actually defines the bufr data structure, so it's a fixed input

! Hardwire some obs - not being fancy here
  nobs = 3                ! number of obs
  ndate = 2018090100      ! analysis/cycle date (YYYYMMDDHH) where HH will be 00, 06, 12, 18
  
  allocate( pres(nobs), &
            uwnd(nobs), &
            vwnd(nobs), &
             lat(nobs), &
             lon(nobs), &
            time(nobs), &   ! note - this is window time (dist. in hours from center of window, e.g. 4:30 is -1.5 from center of 6:00 window)
      wind_error(nobs), &
      pres_error(nobs), &
     wind_qcflag(nobs), &
     pres_qcflag(nobs), &
        obs_type(nobs), &
          sat_id(nobs)   )

!       obs        #1     #2     #3
 pres        = (/444.4, 333.3, 222.2/)
 uwnd        = (/ 44.4,  33.3,  22.2/)
 vwnd        = (/  4.4,   3.3,   2.2/)
 lat         = (/  0.0,   0.0,   0.0/)
 lon         = (/140.7, 285.0,  40.0/) ! note:  lon 0-360
 time        = (/ -1.5,   0.0,   1.5/)
 wind_error  = (/  5.0,   4.0,   3.0/)
 pres_error  = (/ miss,  miss,  miss/)
 wind_qcflag = (/  2.0,   2.0,   2.0/)
 pres_qcflag = (/  2.0,   2.0,   2.0/)
 obs_type    = (/242.0, 247.0, 253.0/) ! kx or Observation_Type 
 sat_id      = (/173.0, 270.0,  70.0/) ! WMO satellite ID:  See COMMON CODE TABLE C–5:Satellite identifier
                                       !    at https://www.wmo.int/pages/prog/www/WMOCodes/WMO306_vI2/LatestVERSION/WMO306_vI2_CommonTable_en.pdf

! open bufr file for write
  call init_bufr(outfn, table_fn)

  do i=1,nobs
      write(*,fmt="(A4,I5,1X,A25,6F8.3)")'Obs=',i,'pres,uwnd,vwnd,lat,lon,time=',pres(i),uwnd(i),vwnd(i),lat(i),lon(i),time(i)
      write(*,fmt="(5X,A62,6F7.1)")'wind_error,pres_error,wind_qcflag,pres_qcflag,obs_type,sat_id=',wind_error(i),pres_error(i),wind_qcflag(i),pres_qcflag(i),obs_type(i),sat_id(i)
      print *, sat_id(i)
      call write_cat6_wind(stationid, lon(i), lat(i), time(i), obs_type(i), pres(i), uwnd(i), vwnd(i), &
           pres_qcflag(i), wind_qcflag(i), pres_error(i), wind_error(i), subset, ndate, miss, sat_id(i) )
!           Note:      elevation, which can be missing for this type (for now at least) ^^^^

  enddo

  call end_bufr
 end program write_prepbufr_example
