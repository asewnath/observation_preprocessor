
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
      module  preprocessor_kma

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

      subroutine get_labels(str_ptr)bind(C, name="get_labels_kma")
      use iso_c_binding
      type(c_ptr), intent(in) :: str_ptr
      character(kind=c_char, len=100), pointer :: f_ptr_str
      call c_f_pointer(str_ptr, f_ptr_str)
      f_ptr_str = 'lon lat time obs_type elv sat_id pres uwnd vwnd'
     .  // ' rffl qify qifn saza eham'C

      end subroutine get_labels

      subroutine proc(data_ptr,int_ptr,
     &      date_ptr,file_ptr,str_length) bind(C, name="data_kma")

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

      integer lunit, luout, lutbl
      character(len=180) :: infile, outfile
      character(len=220) cmtdir, ptbl
      character(len=1) csat(50:59)
      character(len=8) subset, sid
      character(len=10) cymd
      character(len=1) bfmg(MXMSGL)
      integer ibfmg ( MXMSGLD4 )
      real(8) getbmiss, bmiss

      integer obs_count, debug

      EQUIVALENCE ( bfmg(1), ibfmg(1) )

      real(8) hdr(12), extra(1), obs(3), qcdat(2,12), pccf(4)
      real qify, qifn, rffl, eeqf, wqm
      real hgtf, p
      real dhr
      integer idate
      integer ierr, nbyt, iret, iret2, ilev, ilev2
      integer i, ilev3, irt1, irt2, irt3, ilev4
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

      !  open the input file for reading by C I/O interface
      !call cobfl(infile,'r')
      print *, "open the file?"
      !  set up for using master BUFR tables
      !lunit = 92
      !open( unit=lunit, file='/dev/null')
      lunit = 92
      lutbl = 62
      open( unit=lunit, file=infile, form='unformatted')
      open( unit=lutbl, file='kma.bufrtable', form='formatted')


      !  cmtdir = master BUFR tables directory
      cmtdir = '/discover/nobackup/msienkie/tmp/A/debufr.fd'
      !call openbf(lunit,'SEC3',lunit)     ! read using master tables
      !call mtinfo(cmtdir, 90, 91)
      call openbf(lunit,'IN',lutbl)

      call datelen(10)
      bmiss = getbmiss()

      iknt = 0
      ierr = 0
      num  = 0

      !lutbl = 62
      !ptbl = '/discover/nobackup/msienkie/CDAS_prepbufr_tables/'
      !.  // 'gdas_prepbufr_2019061212.tbl'
      !open(unit=lutbl,file=ptbl,form='formatted')

      do while (ireadmg(lunit, subset, IDATE) >= 0)
      !do while ( ierr == 0 )
        !print *, "get here?" is this running through the obs twice?

!  read a BUFR message using the C interface
!        call crbmg( bfmg, MXMSGL, nbyt, ierr)
!        if (ierr /= 0) then
!           call ccbfl()   ! at end or error, close the bufr file
!           exit
!        end if

!  obtain information from the BUFR message via readerme
!        call readerme ( ibfmg, lunit, subset, idate, iret)
!        if (iret .eq. 11) then
!           print *,'BUFR dictionary message?  Skipping'
!           cycle
!        else if (iret < 0) then
!           print *,'unable to read BUFR message'
!           exit
!        end if

!  read all the observations from this BUFR message
        iret2 = 0
        do while (iret2 == 0)
           call readsb( lunit, iret2)
           if (iret2 /= 0) cycle

           call ufbint(lunit,hdr,12,1,ilev,
     . 'SAID YEAR MNTH DAYS HOUR MINU SECO SWCM SAZA SCCF CLATH CLONH' )
           call ufbint(lunit,extra,1,1,ilev4,'EHAM')
!           call ufbint(lunit,hdr,13,1,ilev,
!.'SAID YEAR MNTH DAYS HOUR MINU SECO SWCM SAZA SCCF CLATH CLONH EHAM')


!     crude check for a 3-hr time window
           idate5(1) = hdr(2)  ! year
           idate5(2) = hdr(3)  ! month
           idate5(3) = hdr(4)  ! day
           idate5(4) = hdr(5)  ! hours
           idate5(5) = hdr(6)  ! minutes

!  time check
           call w3fs21(idate5, nmin)
           dhr = (nmin+hdr(7)/3600.-isynmn)/60.
           if (abs(dhr) > 3.) cycle
!  location check
           if (hdr(12) < 0.0) hdr(12) = hdr(12) + 360.
           if (hdr(12) < 0.0 .or. hdr(12) .gt. 360.0) cycle
           if (hdr(11) < -90.0 .or. hdr(11) .gt. 90.0) cycle

!  ityp as assigned by read_satwnd.  May be able to indicate WV cloud-top
!  vs. deep layer by setting ITP in the prepdata file
           wqm = 2.
           if (hdr(8) == 1.0) then
              !ityp = 253                 ! IR winds
              ityp = 261
           else if (hdr(8) == 2.0) then
              !ityp = 243                 ! visible winds
              ityp = 262
           else if (hdr(8) == 3.0) then
              !ityp = 254                 ! WV cloud top
              ityp = 263
           else if (hdr(8) >= 4.0) then
              !ityp = 254                 ! WV deep layer, monitored
              ityp = 264
              wqm = 9.
           end if

!
!  note this only gets the 1st instance of pressure, wind dir & wind speed
!  which I think is what we want...
           call ufbint(lunit,obs,3,1,ilev2,
     .           'PRLC UWND VWND')

           if (obs(1) > 110000. .or. obs(1) .lt. 5000. ) cycle

!  this gets standard generating application (GNAPS)
!  and the PCCF replication associated with GNAPS
           call ufbrep(lunit,qcdat,2,12,ilev3,'GNAPS PCCF')
           rffl = 110        ! GNAPS=3 (but using GNAPS=4 here)
           qify = 110        ! GNAPS=5
           qifn = 110        ! GNAPS=6
           eeqf = 110        ! GNAPS=7
!  check for quality marks based on GNAPS standard generating application
           do i = 1,ilev3
              if (qify<105.and.qifn<105.and.rffl<105.and.eeqf<105) exit
              if ( qcdat(1,i) < bmiss .and. qcdat(2,i) < bmiss) then
                 if(qcdat(1,i)==6.0 .and. qify>105) then
                    qify = qcdat(2,i)    ! QI with forecast
                 else if(qcdat(1,i)==5.0.and.qifn>105) then
                    qifn = qcdat(2,i)    ! QI without forecast
                 else if(qcdat(1,i)==4.0.and.rffl>105) then
                    rffl = qcdat(2,i)    ! actually common QI without fcst
                 else if(qcdat(1,i)==7.0.and.rffl>105) then
                    eeqf = qcdat(2,i)    ! Estimated Error EE
                 end if
              end if
           end do

           phdr = bmiss
           pobs = bmiss
           pccf = bmiss

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
           !print *, lon
           pres = obs(1)*.01
           f_ptr(num + 7) = pres
           !uwnd = obs(3)*cos(obs(2)*deg2rad)
           uwnd = obs(2)
           f_ptr(num + 8) = uwnd
           !vwnd = obs(3)*sin(obs(2)*deg2rad)
           vwnd = obs(3)
           !print *, obs(3)
           f_ptr(num + 9) = vwnd
           f_ptr(num + 10) = rffl
           f_ptr(num + 11) = qify
           f_ptr(num + 12) = qifn
           f_ptr(num + 13) = hdr(9) !SAZA
           f_ptr(num + 14) = extra(1) !EHAM
           num = num + 14
         end do

      end do

      f_ptr_int = obs_count
      call closbf(lunit) !closing bufr manually here
      close(lunit)
      close(lutbl)

      end subroutine proc
      end module preprocessor_kma
