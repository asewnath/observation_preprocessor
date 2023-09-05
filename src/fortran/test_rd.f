      program test_rd
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
      use modv_mxmsgl           ! specifies maximum bufr message size
      
      implicit none
!
! !DESCRIPTION:  example program to EUMETSAT AMV to BUFR
!                using the master BUFR tables
!
! !REVISION HISTORY:
!     10 Oct 2019   Sienkiewicz  Initial version
!     15 Jun 2021   Sienkiewicz  Modified for KMA wind file
!
!EOP
!-----------------------------------------------------------------------

      integer lunit, luout, lutbl
      character(len=180) infile, outfile
      character(len=220) cmtdir, ptbl
      character(len=8) subset, sid
      character(len=10) cymd
      character(len=1) bfmg(MXMSGL)
      integer ibfmg ( MXMSGLD4 )
      real(8) getbmiss, bmiss

      EQUIVALENCE ( bfmg(1), ibfmg(1) )
      
      real(8) hdr(7), obs(3), qcdat(2,12), pccf(4)
      real qify, qifn, rffl, eeqf, wqm
      real hgtf, p
      real dhr
      integer idate
      integer ierr, nbyt, iret, iret2, ilev, ilev2
      integer i, ilev3, irt1, irt2, irt3
      integer idate5(5), isyn, isynmn, nmin, idt, iymd
      integer iknt
      integer ityp

      real(8) phdr(8), pobs(5), pi, deg2rad

! standard atmosphere function from prepdata
      HGTF(P) = (1.-(P/1013.25)**(1./5.256))*(288.15/.0065)
     

      if ( iargc() < 3) then
         print *,'Usage:  test_rd.x YYYYMMDDHH infile outfile '
         stop
      end if
      
      call getarg(1,cymd)       ! synoptic time in format YYYYMMDDHH
      call getarg(2,infile)     ! input file
      call getarg(3,outfile)

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
      call cobfl(infile,'r')

!  set up for using master BUFR tables
      lunit = 92
      open( unit=lunit, file='/dev/null')

!  cmtdir = master BUFR tables directory
      cmtdir = '/discover/nobackup/msienkie/tmp/A/debufr.fd'     
      call openbf(lunit,'SEC3',lunit)     ! read using master tables
      call mtinfo(cmtdir, 90, 91)

      call datelen(10)
      bmiss = getbmiss()

      iknt = 0
      ierr = 0
      luout = 61
      lutbl = 62
      ptbl = '/discover/nobackup/msienkie/CDAS_prepbufr_tables/'
     .  // 'gdas_prepbufr_2019061212.tbl'
      open(unit=luout,file=trim(outfile),form='unformatted')
      open(unit=lutbl,file=ptbl,form='formatted')
      call openbf(luout,'OUT',lutbl)
      
      do while ( ierr == 0 )
        print *, "in here"      
!  read a BUFR message using the C interface
         call crbmg( bfmg, MXMSGL, nbyt, ierr)
         if (ierr /= 0) then
            call ccbfl()   ! at end or error, close the bufr file
            exit
         end if
        
!  obtain information from the BUFR message via readerme
         call readerme ( ibfmg, lunit, subset, idate, iret)
         if (iret .eq. 11) then
            print *,'BUFR dictionary message?  Skipping'
            cycle
         else if (iret < 0) then
            print *,'unable to read BUFR message'
            exit
         end if

!  read all the observations from this BUFR message
         iret2 = 0
         do while (iret2 == 0)
            call readsb( lunit, iret2)
            if (iret2 /= 0) cycle

            call ufbint(lunit,hdr,12,1,ilev,
     . 'SAID YEAR MNTH DAYS HOUR MINU SECO SWCM SAZA SCCF CLATH CLONH' )
            
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
               ityp = 253                 ! IR winds
            else if (hdr(8) == 2.0) then
               ityp = 243                 ! visible winds
            else if (hdr(8) == 3.0) then
               ityp = 254                 ! WV cloud top
            else if (hdr(8) >= 4.0) then
               ityp = 254                 ! WV deep layer, monitored
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
                  if(qcdat(1,i)==5.0 .and. qify>105) then
                     qify = qcdat(2,i)    ! QI with forecast
                  else if(qcdat(1,i)==6.0.and.qifn>105) then
                     qifn = qcdat(1,i)    ! QI without forecast
                  else if(qcdat(1,i)==4.0.and.rffl>105) then
                     rffl = qcdat(3,i)    ! actually common QI without fcst
                  else if(qcdat(1,i)==7.0.and.rffl>105) then
                     eeqf = qcdat(3,i)    ! Estimated Error EE
                  end if
               end if
            end do

            phdr = bmiss
            pobs = bmiss
            pccf = bmiss

            iknt = iknt + 1
            write(sid,'(a1,i7.7)') 'K',iknt     ! just using K in station id
            phdr(1) = transfer(sid,bmiss)
            phdr(2) = hdr(12)
            phdr(3) = hdr(11)
            phdr(4) = dhr
            phdr(5) = ityp
            phdr(6) = hgtf(obs(1)*.01)
            phdr(7) = hdr(1)
            phdr(8) = hdr(9)
            pobs(1) = 6.               ! single level obs
            pobs(2) = obs(1)*.01
            pobs(3) = 2.
            pobs(4) = obs(2)
            pobs(5) = obs(3)
            pobs(6) = wqm
            pccf(1) = rffl
            pccf(2) = qify
            pccf(3) = qifn
            pccf(4) = eeqf
              
!  Satellite zenith angle check
            if (hdr(9) > 68.0) pobs(3) = 14.
! QC screening
!           if (qifn < 85.0) pobs(3) = 14.

            call openmb(luout,'SATWND', iymd)
            call ufbint(luout,phdr,8,1,irt1,
     .           'SID XOB YOB DHR TYP ELV SAID SAZA')
            call ufbint(luout,pobs,6,1,irt2,'CAT POB PQM UOB VOB WQM')
            call ufbint(luout,pccf,4,1,irt3,'RFFL QIFY QIFN EEQF')
            call writsb(luout)
         end do
         
      end do
      call closbf(luout)
      
      stop
      end
