     
!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 610.1   !
!-----------------------------------------------------------------------
!BOP
!
! !ROUTINE: test_rd:  sample read of EUMETSAT AMV and write of prepbufr
!
! !INTERFACE:
!
!     Usage:  test_rd.x YYYYMMDDHH inputfile outputfile
!
! !USES:
!
      module  preprocessor_bufr

      !use, intrinsic:: iso_c_binding
      use modv_mxmsgl           ! specifies maximum bufr message size
      implicit none
!
! !DESCRIPTION:  example program to EUMETSAT AMV to BUFR
!                using the master BUFR tables
!
! !REVISION HISTORY:
!     10 Oct 2019   Sienkiewicz  Initial version
!
!EOP
!-----------------------------------------------------------------------

!Modified cobfl.c in NCEP directory for this to work for multiple bufr files
!Future fix: make a cobfl_preprocess.c file with changes and save it in NCEP?

      CONTAINS

! MODIFIED TO WRITE TO TEXT FILE

      subroutine get_labels(str_ptr)bind(C, name="get_labels_metsat")
      use iso_c_binding
      type(c_ptr), intent(in) :: str_ptr
      character(kind=c_char, len=100), pointer :: f_ptr_str
      call c_f_pointer(str_ptr, f_ptr_str)
      f_ptr_str = 'lon lat time obs_type elv sat_id pres uwnd vwnd'
     .  // ' rffl qify qifn wqcflag'C

      end subroutine get_labels

      subroutine proc(data_ptr,int_ptr,
     &      date_ptr,file_ptr,str_length) bind(C, name="data_metsat")

      use iso_c_binding
      type(c_ptr), intent(in) :: data_ptr
      type(c_ptr), intent(in) :: int_ptr
      type(c_ptr), intent(in) :: date_ptr
      type(c_ptr), intent(in) :: file_ptr
      integer, intent(in) :: str_length
      
      real(kind=c_double), pointer :: f_ptr(:)
      integer(kind=c_int), pointer :: f_ptr_int
      character(kind=c_char, len=10), pointer :: f_ptr_date
      character(kind=c_char, len=str_length), pointer :: f_ptr_file   

      integer num

      integer lunit, lutbl
      character(len=180) :: infile
      character(len=220) cmtdir, ptbl
      character(len=1) csat(50:59)
      character(len=8) subset, sid
      character(len=10) cymd
      character(len=1) bfmg(MXMSGL)
      integer ibfmg ( MXMSGLD4 )
      real(8) getbmiss, bmiss

      integer obs_count, debug

      EQUIVALENCE ( bfmg(1), ibfmg(1) )
      
      real(8) hdr(12), obs(3), qcdat(3,12), pccf(4)
      real qify, qifn, rffl, wqm
      real hgtf, p
      real dhr, md
      integer idate
      integer ierr, nbyt, iret, iret2, ilev, ilev2
      integer i, ilev3, irt1, irt2, irt3
      integer idate5(5), isyn, isynmn, nmin, idt, iymd
      integer iknt
      integer ityp
      real(8) phdr(8), pobs(5), pi, deg2rad

!WM bufr ireadmg define
      integer ireadmg

      real lon, lat, time, obs_type, pres       
      real uwnd, vwnd, ndate, elv, sat_id

      character(300) text_form
      parameter(text_form = '(9(1x, f8.3) )')
      data csat /'Z', 'W', 'X', 'Y', 'Z', 'W', 'X', 'Y', 'Z', 'W'/
 
! standard atmosphere function from prepdata
      HGTF(P) = (1.-(P/1013.25)**(1./5.256))*(288.15/.0065)

      call c_f_pointer(data_ptr, f_ptr, [2])
      call c_f_pointer(int_ptr, f_ptr_int)
      call c_f_pointer(date_ptr, f_ptr_date)
      call c_f_pointer(file_ptr, f_ptr_file)

      obs_count = 0
      debug = 0
      cymd = f_ptr_date
      infile = f_ptr_file

      read(cymd,'(i10)') iymd

!  get minutes from 1/1/78 for synoptic time center
      isyn = iymd
      idate5(1) = isyn/1000000
      isyn = isyn - 1000000 * idate5(1)
      idate5(2) = isyn/10000
      isyn = isyn - 10000 * idate5(2)
      idate5(3) = isyn/100
      isyn = isyn - 100 * idate5(3)
      idate5(4) = isyn
      idate5(5) = 0
      call w3fs21(idate5,isynmn)
      pi      = acos(-1.0)
      deg2rad = pi/180.0

!  set up for using master BUFR tables
      lunit = 92
      lutbl = 62
      open( unit=lunit, file=infile, form='unformatted')
      open( unit=lutbl, file='bufrtab.JAPAN_SATWIND', form='formatted')

!  cmtdir = master BUFR tables directory
      cmtdir = '/discover/nobackup/msienkie/tmp/A/debufr.fd'     
      call openbf(lunit,'IN',lutbl)
      call datelen(10)
      bmiss = getbmiss()
      
      iknt = 0
      ierr = 0
      !luout = 61
      ptbl = '/discover/nobackup/msienkie/CDAS_prepbufr_tables/'
     .  // 'gdas_prepbufr_2019061212.tbl'

      num = 0
      do while(ireadmg(lunit, subset, IDATE) >= 0)

!  read all the observations from this BUFR message
         iret2 = 0
       
         do while (iret2 == 0)
            call readsb( lunit, iret2)

            call ufbint(lunit,hdr,12,1,ilev,
     . 'SAID YEAR MNTH DAYS HOUR MINU SECO SWCM SAZA SCCF CLATH CLONH' )
            

            !if(hdr(11).lt.0)print*, hdr(11)
!     crude check for a 3-hr time window
            idate5(1) = hdr(2)  ! year
            idate5(2) = hdr(3)  ! month
            idate5(3) = hdr(4)  ! day
            idate5(4) = hdr(5)  ! hours
            idate5(5) = hdr(6)  ! minutes

            call w3fs21(idate5, nmin)

            dhr = (nmin+hdr(7)/3600.-isynmn)/60.

            if (abs(dhr) > 3.) cycle
            
            if (hdr(12) < 0.0) hdr(12) = hdr(12) + 360.
            if (hdr(12) < 0.0 .or. hdr(12) .gt. 360.0) cycle
            if (hdr(11) < -90.0 .or. hdr(11) .gt. 90.0) cycle
            if (hdr(9) > 68.0) cycle

            !if(hdr(11).lt.0)print*, hdr(11)
            
!  ityp as assigned by read_satwnd.  May be able to indicate WV cloud-top
!  vs. deep layer by setting ITP in the prepdata file
            wqm = 2.
            if (hdr(8) == 1.0) then
               ityp = 253                 ! IR winds
            else if (hdr(8) == 2.0) then
               ityp = 243                 ! visible winds
            else if (hdr(8) == 3.0) then
               ityp = 254                 ! WV cloud top
            else if (hdr(8) >= 4.0) then
               ityp = 254                 ! WV deep layer, monitored
               wqm = 9. 
            end if
            
            !if(hdr(11).lt.0)print*, hdr(11) !prints
            !print *, hdr(12)
            !debug = debug + 1
            !print *, debug
!
!  note this only gets the 1st instance of pressure, wind dir & wind speed
!  which is what we want... 


            !print*, hdr(1)
            lat = hdr(11)
            !if(lat.lt.0)print*, lat
            f_ptr(num + 2) = lat

            call ufbint(lunit,obs,3,1,ilev2,'PRLC WDIR WSPD')
            !print*, "hdr not printing"
            !print *, obs(1)
            !print *, obs(2)
            !print *, obs(3)
            !print *, hdr(1)
            !print *, lat           
            !if(hdr(11).lt.0)print*, hdr(11) !doesnt print
            if (obs(1) > 110000. .or. obs(1) .lt. 5000. ) cycle

            !if(hdr(11).lt.0)print*, hdr(11)
!  this gets generating center (GCLONG), generating application (GNAP)
!  and the first of the PCCF replication associated with GCLONG and GNAP
            call ufbrep(lunit,qcdat,3,12,ilev3,'GCLONG GNAP PCCF')
            qify = 110
            qifn = 110
            rffl = 110
!  check for quality marks based on GCLONG=254 code table for EUMETSAT
            do i = 4,9
               if ( qify<105 .and. qifn<105 .and. rffl<105) exit
               if ( qcdat(2,i) < bmiss .and. qcdat(3,i) < bmiss) then
                  if(qcdat(2,i)==1.0.and.qcdat(3,i)<bmiss) then
                     qify = qcdat(3,i)    ! QI with forecast
                  else if(qcdat(2,i)==2.0.and.qcdat(3,i)<bmiss) then
                     qifn = qcdat(3,i)    ! QI without forecast
                  else if(qcdat(2,i)==3.0.and.qcdat(3,i)<bmiss) then
                     rffl = qcdat(3,i)    ! NESDIS RFF
                  end if
               end if
            end do
            
            phdr = bmiss
            pobs = bmiss
            pccf = bmiss


          
            !derive mathematical direction
            md = 270 - obs(2)
            if(md < 0) then
              md = md + 360
            end if

            iknt = iknt + 1
            write(sid,'(a1,i7.7)') csat(nint(hdr(1))),iknt
            obs_count = obs_count + 1
            lon = hdr(12)
            f_ptr(num + 1) = lon
            lat = hdr(11)
            !if(lat.lt.0)print*, lat
            f_ptr(num + 2) = lat
            !print *, lon
            time = dhr
            f_ptr(num + 3) = time
            obs_type = ityp
            f_ptr(num + 4) = obs_type
            elv = hgtf(obs(1)*.01)
            f_ptr(num + 5) = elv
            sat_id = hdr(1)
            f_ptr(num + 6) = sat_id
            pres = obs(1)*.01
            f_ptr(num + 7) = pres
            !uwnd = -obs(3)*sin(obs(2)*deg2rad)
            uwnd = obs(3)*cos(md*deg2rad)
            f_ptr(num + 8) = uwnd
            !vwnd = -obs(3)*cos(obs(2)*deg2rad)
            vwnd = obs(3)*sin(md*deg2rad)
            f_ptr(num + 9) = vwnd
            f_ptr(num + 10) = rffl
            f_ptr(num + 11) = qify
            f_ptr(num + 12) = qifn
            f_ptr(num + 13) = wqm
            num = num + 13
         end do

      end do

      f_ptr_int = obs_count
      call closbf(lunit) !closing bufr manually here
      close(lunit)
      close(lutbl)

      end subroutine proc
      end module preprocessor_bufr


