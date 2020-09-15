!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !MODULE:  m_errtbl --- routines to read and assign NCEP obs errors
!
! !INTERFACE:
!

      module  m_errtbl
      
! !USES:
     
      use m_ioutil, only : luavail
      use m_die
      
      implicit none
      
! !DESCRIPTION:
!     
!    collection of routines to read the NCEP obs error table and
!    return obs error values for various obs types
!
! !REVISION HISTORY:
!
!  27Apr2004   Meta    Initial code
!
!EOP
!-------------------------------------------------------------------------

      private
      public :: init_etable, getoe

      real(8) :: missing = 10.e10
      
      real       ERRS(300,33,6)
      
      CONTAINS   

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: init_etable ---  open error table file and read in data
!
! !INTERFACE:

      subroutine init_etable(tablefile)    !  optional
      
! !USES:
     
      implicit none
      
      
! !INPUT PARAMETERS

      character(len=*), optional :: tablefile ! BUFR table file
      
! !DESCRIPTION:
!     
!    Open user specified file, and read data into error array
!
! !REVISION HISTORY:
!
!  27Apr2004   Meta    Initial code (from GBLEVN01)
!
!EOP
!-------------------------------------------------------------------------

      character(len=200) errtable
      integer             iunite
      integer             irec
      integer             kx
      integer k, m
      
      if (present(tablefile)) then
         errtable = tablefile
      else
         errtable="prepobs_errtable.global"
      endif

! find unit numbers for files
      iunite = luavail()
      open(unit=iunite,file=errtable,action='read')

      REWIND IUNITE

      IREC = 0

   10 CONTINUE
      READ(IUNITE,'(1X,I3)',END=100) KX
      IREC = IREC + 1
      DO K=1,33
         READ(IUNITE,'(1X,6E12.5)') (ERRS(KX,K,M),M=1,6)
      ENDDO
      GO TO 10

  100 CONTINUE
      IF(IREC.EQ.0) THEN
         call die('etable', 'OBS. ERROR TABLE EMPTY OR DOES NOT EXIST', 60)
      END IF
      
      close(iunite)

      RETURN

      end subroutine init_etable

!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: getoe ---  get obs error for a report
!
! !INTERFACE:

      subroutine getoe(pob,cat,typ,poe,
     &          qoe, toe, woe, pwe)
      
! !USES:
     
      implicit none
      
! !INPUT PARAMETERS

      real                pob               ! observation pressure
      real                cat               ! data category
      real                typ               ! obs report type

! !OUTPUT PARAMETERS
      real                poe               ! pressure obs error
      real, optional ::   qoe               ! spec. hum. obs error
      real, optional ::   toe               ! temperature obs error
      real, optional ::   woe               ! wind obs error
      real, optional ::   pwe               ! precip. water obs error
      
                  
! !DESCRIPTION:
!     
!   Get obs errors for different data types at the specified pressure level
!
! !REVISION HISTORY:
!
!  27Apr2004   Meta    Initial code adapted from gblevents
!
!EOP
!-------------------------------------------------------------------------

      poe = missing
      if (cat .eq. 0)  poe = OEFG01(POB,TYP,5)
      
      if (present(qoe)) qoe = OEFG01(POB,TYP,3)     ! specific humidity
      if (present(toe)) toe = OEFG01(POB,TYP,2)     ! temperature
      if (present(woe)) woe = OEFG01(POB,TYP,4)     ! wind
      if (present(pwe)) pwe = OEFG01(POB,TYP,6)     ! precipitable water

      return
      
      end subroutine getoe
      
!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!      NASA/GSFC, Global Modeling and Assimilation Office, Code 900.3   !
!-----------------------------------------------------------------------
!BOP
!
! !IROUTINE: oefg01 ---  look up errors for ob types
!
! !INTERFACE:

      real FUNCTION OEFG01(P,TYP,IE) ! FORMERLY FUNCTION OEF
      
! !USES
      implicit none
      
! !INPUT PARAMETERS
      real           p        ! pressure level
      real           typ      ! report type
      integer        ie       ! ob type

! !DESCRIPTION:
!     
!   Look up errors for particular obs types using NCEP code.
!
! !REVISION HISTORY:
!
!  27Apr2004   Meta    Initial code adapted from gblevents
!
!EOP
!-------------------------------------------------------------------------
      integer     kx           ! integer obs type
      integer     la           ! loop value (level above)
      integer     lb           ! level below
      real        del          ! weight


      OEFG01 = missing
      KX  = TYP

C  LOOK UP ERRORS FOR PARTICULAR OB TYPES
C  --------------------------------------

      IF(IE.GE.2 .AND. IE.LE.4) THEN
         DO LA=1,33
            IF(P.GE.ERRS(KX,LA,1)) GOTO 10
         ENDDO
   10    CONTINUE
         LB = LA-1
         IF(LB.EQ.33) LA = 6
         IF(LB.EQ.33) LB = 5
         IF(LB.EQ. 0) THEN
            OEFG01 = ERRS(KX,1,IE)
         ELSE
            DEL = (P-ERRS(KX,LB,1))/(ERRS(KX,LA,1)-ERRS(KX,LB,1))
            OEFG01 = (1.-DEL)*ERRS(KX,LB,IE) + DEL*ERRS(KX,LA,IE)
         ENDIF
      ELSEIF(IE.EQ.5) THEN
         OEFG01 = ERRS(KX,1,5)
      ELSEIF(IE.EQ.6) THEN
         OEFG01 = ERRS(KX,1,6)
      ENDIF

C  SET MISSING ERROR VALUE TO 10E10
C  --------------------------------

      IF(OEFG01.GE.5E5) OEFG01 = missing

      RETURN
      END function oefg01
      
      end module m_errtbl
