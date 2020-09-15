!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !MODULE:  m_pbmin --- routines to write minimal PREPBUFR
!
! !INTERFACE:
!

      module  c_pbmin
      
! !USES:
     
      use c_pbutil
      implicit none
      
! !DESCRIPTION:
!     
!    collection of routines to write smaller amount of PREPBUFR,
!    what is actually read in by SSI/GSI routine "read_prepbufr"
!
!    note will need to modify this module as GSI code changes!
!
!    zob, zqm only for cat 0 (surface height, `ps' obs)
!
!    subset is read in, but not used by GSI
!
! !REVISION HISTORY:
!
!  06Apr2004   Meta    Initial code
!  16Apr2004   Meta    Added i_bfr, r_bfr related definitions to handle
!                       calling BUFRLIB compiled with different options
!                       (specifically, -i8) than the module.
!   8Nov2004   Meta    Moved common BUFR utilities (open, close) to m_pbutil
!  
!
!EOP
!-------------------------------------------------------------------------
      integer, parameter :: MXBLVL = 255 ! max no. of report levels allowed


      real(8), dimension(8) ::    hdr    !  observation header
      real(8), dimension(8,255):: drfdat !  balloon drift data
      real(8), dimension(8,255):: obsdat !  observation data
      real(8), dimension(8,255):: qcmark !  qc marks
      real(8), dimension(8,255):: obserr !  observation errors
      
      real(8) :: rid                     ! real*8 variable
      
      integer(i_bfr), parameter :: iarr = 8    ! size of bfr arrays
      integer(i_bfr), parameter :: i1   = 1    ! single level
      integer(i_bfr), parameter :: ilv = MXBLVL  ! multiple level
      
      integer(i_bfr) iret                !  return code
      integer(i_bfr) ibdate              !  synoptic date/time
      integer(i_bfr) ilevs               !  no. of levels used
      

      character(len=40) obstr,drift,hdstr,qcstr,oestr
      data hdstr  /'SID XOB YOB DHR TYP ELV SAID'/
      data obstr  /'POB QOB TOB ZOB UOB VOB PWO CAT '/
      data drift  /'XDR YDR HRDR                    '/
      data qcstr  /'PQM QQM TQM ZQM WQM NUL PWQ     '/
      data oestr  /'POE QOE TOE NUL WOE NUL PWE     '/

      CONTAINS   

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_upa_mass ---  write multilevel mass report (e.g. raob)
!
! !INTERFACE:

      subroutine write_upa_mass(stnid, xob, yob, dhr, typ, nlev,
     &   pob, qob, tob, zob, cat, pqm, qqm, tqm, zqm, poe, qoe, toe, 
     &   subset, idate, xdr, ydr, hrdr)
      
      
! !INPUT PARAMETERS
      character(len=8),intent(in) ::  stnid            ! station identifier
      real,intent(in) ::              xob              ! longitude (0-360)
      real,intent(in) ::              yob              ! latitude
      real,intent(in) ::              dhr              ! delta obs time from synoptic
      real,intent(in) ::              typ              ! report type
      integer,intent(in) ::           nlev             ! number of levels
      real,intent(in) ::              pob(nlev)        ! pressure
      real,intent(in) ::              qob(nlev)        ! specific humidity
      real,intent(in) ::              tob(nlev)        ! temperature
      real,intent(in) ::              zob(nlev)        ! geopotential height 
      real,intent(in) ::              cat(nlev)        ! data category for level
      real,intent(in) ::              pqm(nlev)        ! pressure quality mark
      real,intent(in) ::              qqm(nlev)        ! humidity quality mark
      real,intent(in) ::              tqm(nlev)        ! temperature quality mark
      real,intent(in) ::              zqm(nlev)        ! height quality mark
      real,intent(in) ::              poe(nlev)        ! pressure observation error
      real,intent(in) ::              qoe(nlev)        ! humidity observation error
      real,intent(in) ::              toe(nlev)        ! temperature observation error
      character(len=8),intent(in) ::  subset           ! observation source
      integer,intent(in) ::           idate            ! synoptic date/time
      real, optional,intent(in)    :: xdr(nlev)        ! balloon drift longitude
      real, optional,intent(in)    :: ydr(nlev)        ! balloon drift latitude
      real, optional,intent(in)    :: hrdr(nlev)       ! drift type minus cycle time
            
! !DESCRIPTION:
!     
!    Write a mass report with multiple levels, such as a raob report.
!    Includes mnemonics for balloon drift (if supplied).  User is
!    responsible for sorting and aligning obs in the pob, qob, tob, etc.
!    arrays.
!
!  Note: need to see if Z is used in upperair processing in SSI or OIQC
!  In SSI/GSI Z is used to process cat0 obs for `ps' processing, it is
!    not used for levels with other categories.  Also in OIQC - ZOB only
!    accessed for `PCHK' processing (tag .eq. `SFC')
!
! !REVISION HISTORY:
!
!  12Apr2004   Meta    Initial skeleton code
!   6May2004   Meta    filled in code for writing to prepbufr
!  15Feb2005   Meta    fix error in header write
!
!EOP
!-------------------------------------------------------------------------

      
      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      drfdat = missing

      if (present(xdr) .and. present(ydr) .and. present(hrdr)) then
        drfdat(1,1:nlev) = xdr(1:nlev)
        drfdat(2,1:nlev) = ydr(1:nlev)
        drfdat(3,1:nlev) = hrdr(1:nlev)
      endif

      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      obsdat(1,1:nlev) = pob(1:nlev)
      obsdat(2,1:nlev) = qob(1:nlev)
      obsdat(3,1:nlev) = tob(1:nlev)
      obsdat(4,1:nlev) = zob(1:nlev)
      obsdat(8,1:nlev) = cat(1:nlev)

      qcmark(1,1:nlev) = pqm(1:nlev)
      qcmark(2,1:nlev) = qqm(1:nlev)
      qcmark(3,1:nlev) = tqm(1:nlev)
      
      obserr(1,1:nlev) = poe(1:nlev)
      obserr(2,1:nlev) = qoe(1:nlev)
      obserr(3,1:nlev) = toe(1:nlev)
      
      ibdate = idate
      ilevs = nlev
      
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1   , iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, ilevs, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, ilevs, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, ilevs, iret, oestr)
      
      call writsb(lu_b)
      
      return
      
      end subroutine write_upa_mass
      
!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_upa_wind ---  write multilevel wind report (e.g. raob)
!
! !INTERFACE:

      subroutine write_upa_wind(stnid, xob, yob, dhr, typ, nlev,
     &   pob, uob, vob, cat, pqm, wqm, poe, woe,
     &   subset, idate, xdr, ydr, hrdr)
      
            
! !INPUT PARAMETERS
      character(len=8),intent(in) ::  stnid        ! station identifier
      real,intent(in) ::              xob          ! longitude (0-360)
      real,intent(in) ::              yob          ! latitude
      real,intent(in) ::              dhr          ! delta obs time from synoptic
      real,intent(in) ::              typ          ! report type
      integer,intent(in) ::           nlev         ! number of levels
      real,intent(in) ::              pob(nlev)    ! pressure
      real,intent(in) ::              uob(nlev)    ! east-west wind component
      real,intent(in) ::              vob(nlev)    ! north-south wind component
      real,intent(in) ::              cat(nlev)    ! data category for level
      real,intent(in) ::              pqm(nlev)    ! pressure quality mark
      real,intent(in) ::              wqm(nlev)    ! wind quality mark
      real,intent(in) ::              poe(nlev)    ! pressure observation error
      real,intent(in) ::              woe(nlev)    ! wind observation error
      character(len=8),intent(in) ::  subset       ! observation source
      integer,intent(in) ::           idate        ! synoptic date/time
      real, optional,intent(in) ::    xdr(nlev)    ! balloon drift longitude
      real, optional,intent(in) ::    ydr(nlev)    ! balloon drift latitude
      real, optional,intent(in) ::    hrdr(nlev)   ! drift type minus cycle time

! !DESCRIPTION:
!     
!    Write a wind report with multiple levels, such as a raob report.
!    Includes mnemonics for balloon drift (if supplied)
!
! !REVISION HISTORY:
!
!  12Apr2004   Meta    Initial skeleton code
!   6May2004   Meta    filled in code for writing to prepbufr
!  10May2004   Meta    Removed ELV mnemonic as it is not needed
!  15Feb2005   Meta    fix error in header write
!
!EOP
!-------------------------------------------------------------------------
      
      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      drfdat = missing

      if (present(xdr) .and. present(ydr) .and. present(hrdr)) then
        drfdat(1,1:nlev) = xdr(1:nlev)
        drfdat(2,1:nlev) = ydr(1:nlev)
        drfdat(3,1:nlev) = hrdr(1:nlev)
      endif

      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      obsdat(1,1:nlev) = pob(1:nlev)
      obsdat(2,1:nlev) = uob(1:nlev)
      obsdat(3,1:nlev) = vob(1:nlev)
      obsdat(8,1:nlev) = cat(1:nlev)

      qcmark(1,1:nlev) = pqm(1:nlev)
      qcmark(5,1:nlev) = wqm(1:nlev)
      
      obserr(1,1:nlev) = poe(1:nlev)
      obserr(5,1:nlev) = woe(1:nlev)
      
      ibdate = idate
      ilevs = nlev
      
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1   , iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, ilevs, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, ilevs, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, ilevs, iret, oestr)
      
      call writsb(lu_b)
      
      end subroutine write_upa_wind

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_cat6_mass ---  write single upper level mass report
!
! !INTERFACE:

      subroutine write_cat6_mass(stnid, xob, yob, dhr, typ,
     &      pob, qob, tob, pqm, qqm, tqm, poe, qoe, toe, subset, idate)
      
      
! !INPUT PARAMETERS
      character(len=8),intent(in) ::  stnid            ! station identifier
      real,intent(in) ::              xob              ! longitude (0-360)
      real,intent(in) ::              yob              ! latitude
      real,intent(in) ::              dhr              ! delta obs time from synoptic
      real,intent(in) ::              typ              ! report type
      real,intent(in) ::              pob              ! pressure
      real,intent(in) ::              qob              ! specific humidity
      real,intent(in) ::              tob              ! temperature
      real,intent(in) ::              pqm              ! pressure quality mark
      real,intent(in) ::              qqm              ! humidity quality mark
      real,intent(in) ::              tqm              ! temperature quality mark
      real,intent(in) ::              poe              ! pressure observation error
      real,intent(in) ::              qoe              ! humidity observation error
      real,intent(in) ::              toe              ! temperature observation error
      character(len=8),intent(in) ::  subset           ! observation source
      integer,intent(in) ::           idate            ! synoptic date/time

                  
! !DESCRIPTION:
!     
!    Write a single level upperair mass report, such as ACARS temperature
!    Name `cat6' refers to ON29 category 6 `Single level data'
!    Note the missing data value is 'missing' (10.e10) 
!
!    Note that poe, qoe, toe can be set missing if you intend to fill 
!    those in using a subsequent program (such as `prevents')
!
! !REVISION HISTORY:
!
!  12Apr2004   Meta    Initial code
!   6May2004   Meta    Removed ELV mnemonic as it is not needed
!
!EOP
!-------------------------------------------------------------------------

      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      
      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      obsdat(1,1) = pob
      obsdat(2,1) = qob
      obsdat(3,1) = tob
      obsdat(8,1) = 6.

      qcmark(1,1) = pqm
      qcmark(2,1) = qqm
      qcmark(3,1) = tqm
      
      obserr(1,1) = poe
      obserr(2,1) = qoe
      obserr(3,1) = toe
      
      ibdate = idate
      
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1, iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, i1, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, i1, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, i1, iret, oestr)
      
      call writsb(lu_b)
      
      return
      
      end subroutine write_cat6_mass

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_cat6_wind ---  write single upper level wind report
!
! !INTERFACE:


      subroutine write_cat6_wind(c_stnid, xob, yob, dhr, typ,
     &      pob, uob, vob, pqm, wqm, poe, woe, c_subset, idate,
     &      elv, satid, rffl, qify, qifn, eeqf) 
     &      bind(c, name="obs_write_wind") 
      
      integer :: i
      character*8 :: stnid
      character*8 :: subset
! !INPUT PARAMETERS
      character(kind=C_CHAR),intent(in) ::  c_stnid (*)            ! station identifier
      real*8,intent(in) ::              xob              ! longitude (0-360)
      real*8,intent(in) ::              yob              ! latitude
      real*8,intent(in) ::              dhr              ! delta obs time from synoptic
      real*8,intent(in) ::              typ              ! report type
      real*8,intent(in) ::              pob              ! pressure
      real*8,intent(in) ::              uob              ! east-west wind component
      real*8,intent(in) ::              vob              ! north-south wind component
      real*8,intent(in) ::              pqm              ! pressure quality mark
      real*8,intent(in) ::              wqm              ! wind quality mark
      real*8,intent(in) ::              poe              ! pressure observation error
      real*8,intent(in) ::              woe              ! wind observation error
      character(kind=C_CHAR),intent(in) :: c_ subset (*)           ! observation source
      integer,intent(in) ::           idate            ! synoptic date/time
      real*8, optional, intent(in)   :: elv              ! elevation  (Above ground level?)
      real*8,optional,intent(in)     :: satid            ! satellite ID    
      real*8, optional, intent(in)   :: rffl             ! recursive filter flag
      real*8, optional, intent(in)   :: qify             ! QI derived with forecast field
      real*8, optional, intent(in)   :: qifn             ! QI derived without forecast field
      real*8, optional, intent(in)   :: eeqf             ! % confidence based on NESDIS expected error


! !DESCRIPTION:
!     
!    Write a single level wind report.  Name `cat6' refers to
!    ON29 category 6 `Single level data'.
!    Note the missing data value is `missing' (10.e10) 
!
!    Note that poe, woe can be set missing if you intend to fill those in
!    using a subsequent program (such as `prevents')
!
!    ELV is not used except for report types 280-289 (surface wind),
!    otherwise this can be set to `missing'
!
! !REVISION HISTORY:
!
!  12Apr2004   Meta    Initial code
!  10May2004   Meta    Scope now includes surface level cat 6 wind obs.
!                      ELV is now an optional argument.
!  27Apr2020   Meta    Add satellite QC information
!EOP
!-------------------------------------------------------------------------

! 'hardwired' elevations
! -----------------------

      real, dimension(280:289), parameter :: hwelv = 
     &     (/ 20., 10., 20., 20., 10., 10., 10., 10., 10., 10. /)

      integer(i_bfr), parameter :: nqc = 4
      real*8  :: satqc(8)

      forall(i=1:8) stnid(i:i) = c_stnid(i)   
      forall(i=1:8) subset(i:i) = c_subset(i)  

      !Input isn't working because of the string to char arr conversion 
      subset = 'SATWND'

      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      satqc  = missing
      
      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      if (typ >= 280. .and. typ < 290.) then
        if (present(elv)) then
           hdr(6) = elv
        else
           hdr(6) = hwelv(nint(typ))
        endif
      endif

      if (present(satid)) then
         hdr(7) = satid
      endif

      if (present(rffl)) then
         satqc(1) = rffl
      end if

      if (present(qify)) then
         satqc(2) = qify
      end if

      if (present(qifn)) then
         satqc(3) = qifn
      end if

      if (present(eeqf)) then
         satqc(4) = eeqf
      end if

      obsdat(1,1) = pob
      obsdat(5,1) = uob
      obsdat(6,1) = vob
      obsdat(8,1) = 6.
      
      qcmark(1,1) = pqm
      qcmark(5,1) = wqm
      
      obserr(1,1) = poe
      obserr(5,1) = woe
      
      ibdate = idate
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1, iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, i1, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, i1, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, i1, iret, oestr)

      if (present(rffl) .or. present(qify) .or. 
     &    present(qifn) .or. present(eeqf) ) then
        call ufbint(lu_b, satqc, nqc, i1, iret, 'RFFL QIFY QIFN EEQF')
      end if
      
      call writsb(lu_b)

      return
      
      end subroutine write_cat6_wind

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_sfc_mass ---  write surface level mass report
!
! !INTERFACE:

      subroutine write_sfc_mass(stnid, xob, yob, dhr, typ, 
     &    pob, qob, tob, zob, pqm, qqm, tqm, zqm, poe, qoe, toe,
     &    subset, idate)
      

! !INPUT PARAMETERS
      character(len=8),intent(in) ::  stnid            ! station identifier
      real,intent(in) ::              xob              ! longitude (0-360)
      real,intent(in) ::              yob              ! latitude
      real,intent(in) ::              dhr              ! delta obs time from synoptic
      real,intent(in) ::              typ              ! report type
      real,intent(in) ::              pob              ! pressure
      real,intent(in) ::              qob              ! specific humidity
      real,intent(in) ::              tob              ! temperature
      real,intent(in) ::              zob              ! height of observation
      real,intent(in) ::              pqm              ! pressure quality mark
      real,intent(in) ::              qqm              ! humidity quality mark
      real,intent(in) ::              tqm              ! temperature quality mark
      real,intent(in) ::              zqm              ! height quality mark
      real,intent(in) ::              poe              ! pressure observation error
      real,intent(in) ::              qoe              ! humidity observation error
      real,intent(in) ::              toe              ! temperature observation error
      character(len=8),intent(in) ::  subset           ! observation source
      integer,intent(in) ::           idate            ! synoptic date/time

            
! !DESCRIPTION:
!     
!    Write a surface level mass report.  This has temperature and will
!    also be where surface pressure (with z level) data is processed
!
! !REVISION HISTORY:
!
!  12Apr2004   Meta    Initial skeleton code
!  10May2004   Meta    Fill in code from write_cat6_mass
!
!EOP
!-------------------------------------------------------------------------

      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      
      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      obsdat(1,1) = pob
      obsdat(2,1) = qob
      obsdat(3,1) = tob
      obsdat(4,1) = zob
      obsdat(8,1) = 0.

      qcmark(1,1) = pqm
      qcmark(2,1) = qqm
      qcmark(3,1) = tqm
      qcmark(4,1) = zqm
      
      obserr(1,1) = poe
      obserr(2,1) = qoe
      obserr(3,1) = toe
      
      ibdate = idate
      
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1, iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, i1, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, i1, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, i1, iret, oestr)
      
      call writsb(lu_b)
      
      return
            
      end subroutine write_sfc_mass

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_tpw ---  write TPW report
!
! !INTERFACE:

      subroutine write_tpw(stnid, xob, yob, dhr, typ,
     &      pwo, pwq, pwe, subset, idate)
      
      
! !INPUT PARAMETERS
      character(len=8),intent(in) ::  stnid            ! station identifier
      real,intent(in) ::              xob              ! longitude (0-360)
      real,intent(in) ::              yob              ! latitude
      real,intent(in) ::              dhr              ! delta obs time from synoptic
      real,intent(in) ::              typ              ! report type
      real,intent(in) ::              pwo              ! precipitable water (PW)
      real,intent(in) ::              pwq              ! PW quality mark
      real,intent(in) ::              pwe              ! PW observation error
      character(len=8),intent(in) ::  subset           ! observation source
      integer,intent(in) ::           idate            ! synoptic date/time

                  
! !DESCRIPTION:
!     
!    Write a TPW report
!
! !REVISION HISTORY:
!
!  11May2004   Meta    Initial code 
!
!EOP
!-------------------------------------------------------------------------

      hdr    = missing
      obsdat = missing
      qcmark = missing
      obserr = missing
      
      hdr(1) = transfer( stnid, rid )
      hdr(2) = xob
      hdr(3) = yob
      hdr(4) = dhr
      hdr(5) = typ
      
      obsdat(7,1) = pwo
      obsdat(8,1) = 6.

      qcmark(7,1) = pwq
      
      obserr(7,1) = pwe
      
      ibdate = idate
      
      call openmb(lu_b, subset, ibdate)

      call ufbint(lu_b, hdr,    iarr, i1, iret, hdstr)
      call ufbint(lu_b, obsdat, iarr, i1, iret, obstr)
      call ufbint(lu_b, qcmark, iarr, i1, iret, qcstr)
      call ufbint(lu_b, obserr, iarr, i1, iret, oestr)
      
      call writsb(lu_b)
      
      return
      
      end subroutine write_tpw

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: write_ozone ---  write sbuv ozone (not PREPBUFR)
!
! !INTERFACE:

      subroutine write_ozone(idate, said, clat, clon, year, month, day,
     &      hour, minute, ozoprof, totozo)
      
      
! !INPUT PARAMETERS
      integer,intent(in) ::   idate               ! synoptic date/time
      integer,intent(in) ::   said                ! satellite id
      real, intent(in)   ::   clat                ! latitude of observation
      real, intent(in)   ::   clon                ! longitude of observation
      integer,intent(in) ::   year                ! year of observation
      integer,intent(in) ::   month               ! month of observation
      integer,intent(in) ::   day                 ! day of observation
      integer,intent(in) ::   hour                ! hour of observation
      integer,intent(in) ::   minute              ! minute of observation
      real,intent(in),dimension(12)  ::   ozoprof ! ozone profile 
      real,intent(in)    ::   totozo              ! total ozone

                  
! !DESCRIPTION:
!     
!    Write a SBUV record with data used by GSI 'read_ozone.f'
!
! !REVISION HISTORY:
!
!  20Oct2004   Meta    Initial code 
!  29Oct2004   Meta    Minor bugfix - did not set ibdate before.
!  
!
!EOP
!-------------------------------------------------------------------------

      integer(i_bfr), parameter :: hdrlen = 8    ! size of header array
      integer(i_bfr), parameter :: nloz_8 = 12    ! size of ozone array
      integer       , parameter :: nloz = 12    ! size of ozone array
      character(63) lozstr
      character(40) ozstr
      data lozstr 
     & /'OSP12 OSP11 OSP10 OSP9 OSP8 OSP7 OSP6 OSP5 OSP4 OSP3 OSP2 OSP1 '/
      data ozstr  /'SAID CLAT CLON YEAR MNTH DAYS HOUR MINU ' /
      real(8),dimension(nloz):: ozone_8
      real(8),dimension(8):: hdroz
      real(8) totoz


!      SAID     SATELLITE IDENTIFIER *
!      CLAT     LATITUDE(COARSE ACCURACY) *
!      CLON     LONGITUDE (COARSE ACCURACY) *
!      YEAR     YEAR *
!      MNTH     MONTH *
!      DAYS     DAY *
!      HOUR     HOUR *
!      MINU     MINUTES *
!      OSP1     Solution PROFILE LAYER OZONE 12  .25-.12MB * ( M ATM CM )
!      OSP2     Solution PROFILE LAYER OZONE 11  .5-.25MB *  ( M ATM CM )
!      OSP3     Solution PROFILE LAYER OZONE 10  1-.5MB *    ( M ATM CM )
!      OSP4     Solution PROFILE LAYER OZONE 9  2-1MB *      ( M ATM CM )
!      OSP5     Solution PROFILE LAYER OZONE 8  4-2MB *      ( M ATM CM )
!      OSP6     Solution PROFILE LAYER OZONE 7  8-4MB *      ( M ATM CM )
!      OSP7     Solution PROFILE LAYER OZONE 6  16-82MB *    ( M ATM CM )
!      OSP8     Solution PROFILE LAYER OZONE 5  32-16MB *    ( M ATM CM )
!      OSP9     Solution PROFILE LAYER OZONE 4  64-32MB *    ( M ATM CM )
!      OSP10    Solution PROFILE LAYER OZONE 3  127-64MB *   ( M ATM CM )
!      OSP11    Solution PROFILE LAYER OZONE 2  250-127MB *  ( M ATM CM )
!      OSP12    Solution PROFILE LAYER OZONE 1  1013-250MB * ( M ATM CM )
!      OTSP     TOT OZONE FOR SOLUTION PROFILE *             ( M ATM CM )


! SAID not actually used by GSI
! does read/assign the 12 levels of ozone listed. 

      totoz = totozo
      hdroz(1) = float(said)
      hdroz(2) = clat
      hdroz(3) = clon
      hdroz(4) = float(year)
      hdroz(5) = float(month)
      hdroz(6) = float(day)
      hdroz(7) = float(hour)
      hdroz(8) = float(minute)

      ozone_8 = ozoprof

      ibdate = idate

      call openmb(lu_b, 'NC008010', ibdate)
      call ufbint(lu_b, hdroz,  hdrlen, i1, iret, ozstr)
      call ufbint(lu_b, ozone_8,  nloz_8, i1, iret, lozstr)
      call ufbint(lu_b,totoz,i1,i1,iret,'OTSP')
      
      call writsb(lu_b)

      end subroutine write_ozone

      end module c_pbmin
