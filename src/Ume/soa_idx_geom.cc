/*!
  \file soa_idx_grad.cc
*/

#include "soa_idx_grad.hh"

namespace Ume {
namespace SOA_Idx {

void Sides::calc_vol() {}

/*
subroutine VolSideCart3(geometry,px,fx,zx,&
     kksll,kksl,kstyp,SSV)
  use delfi_constants, only: third
  integer :: kksll,kksl
  integer :: kstyp(kksll)

  ! *** 3D ***

  integer :: s,z,f,k
  real(8)&
       px(kk3ll,kkpll),   ssv(kk1ll,kksll),&
       zx(kk3ll,kkzll),   fx(kk3ll,kkfll)

  real(8), allocatable, dimension(:,:):: sura

  access /geometry/ kk1ll,kk3ll,kkpll,kkzll,kkfll,&
       kk1v,&
       kksp1(kksll),   kksp2(kksll),   kksz(kksll),&
       kksf(kksll)

  allocate(sura(kk3ll,kksll))

  call F3SURF(1,kk3ll,kksll,kksl,kstyp,SURA,&
       kksp2,px,&
       kksp1,px,&
       kksz, zx)

  do s=1,kksl
     if (kstyp(s) > 0) then
        z =kksz(s)
        f =kksf(s)
        ssv(kk1v,s)= third*( (fx(1,f) - zx(1,z)) * sura(1,s) +&
             (fx(2,f) - zx(2,z)) * sura(2,s) +&
             (fx(3,f) - zx(3,z)) * sura(3,s) )
     endif
  enddo      !s

  deallocate(sura)

  !     ----------
  !     Volume of a tetrahedron
  !     ssv = (f-z) o [(p2-z) x (p1-z)/2] / 3
  !     ssv = (f-z) o surf(z21) / 3
  !     ----------
end subroutine VolSideCart3
*/

/*
subroutine F3SURF(isign,kk3ll,kksll,kksl,kstyp,SURF,&
     kksp1,px1,&
     kksp2,px2,&
     kksp0,px0)

  ! Calculates surface normal for a triangle
  ! surf(012) = (1-0) x (2-0) / 2

  !  0  Normal out of paper
  ! / \
  !/   \
  ! 1-----2

  integer :: s,p0,p2,p1,k
  real(8) :: hfac

  integer :: isign,kk3ll,kksll,kksl,kstyp(kksll),&
       kksp0(kksll),kksp1(kksll),kksp2(kksll)
  real(8)&
       SURF(kk3ll,kksll),&
       px0(kk3ll,*),px1(kk3ll,*),px2(kk3ll,*)

  real(8) :: px01, px02, px03
  real(8) :: s10_1, s10_2, s10_3
  real(8) :: s20_1, s20_2, s20_3

  !alloc s10(kk3ll,kksll),s20(kk3ll,kksll)

  ! code expects this by definition
  ! routines are hard-coded for 3 dims
  if (kk3ll /= 3) then
     call Fatal('F3SURF: kk3ll not 3')
  endif

  ! INTERIOR +1
  if (isign > 0) then
     do s=1,kksl

        if (kstyp(s) > 0) then

           p0=kksp0(s)
           p2=kksp2(s)
           p1=kksp1(s)

           px01 = px0(1, p0)
           px02 = px0(2, p0)
           px03 = px0(3, p0)

           s10_1 = px1(1, p1) - px01
           s10_2 = px1(2, p1) - px02
           s10_3 = px1(3, p1) - px03

           s20_1 = px2(1, p2) - px01
           s20_2 = px2(2, p2) - px02
           s20_3 = px2(3, p2) - px03

           surf(1,s) = 0.5d0*( s10_2*s20_3 - s10_3*s20_2 )
           surf(2,s) = 0.5d0*( s10_3*s20_1 - s10_1*s20_3 )
           surf(3,s) = 0.5d0*( s10_1*s20_2 - s10_2*s20_1 )

        endif

     enddo


     ! BOUNDARY -1

  elseif (isign == -1) then

     do s=1,kksl

        if (kstyp(s) < 0) then

           p0=kksp0(s)
           p2=kksp2(s)
           p1=kksp1(s)

           px01 = px0(1, p0)
           px02 = px0(2, p0)
           px03 = px0(3, p0)

           s10_1 = px1(1, p1) - px01
           s10_2 = px1(2, p1) - px02
           s10_3 = px1(3, p1) - px03

           s20_1 = px2(1, p2) - px01
           s20_2 = px2(2, p2) - px02
           s20_3 = px2(3, p2) - px03

           surf(1,s) = 0.25d0*( s10_2*s20_3 - s10_3*s20_2 )
           surf(2,s) = 0.25d0*( s10_3*s20_1 - s10_1*s20_3 )
           surf(3,s) = 0.25d0*( s10_1*s20_2 - s10_2*s20_1 )

        endif
     enddo
  endif

end subroutine F3SURF
*/

void calc_ccv() {
  /*
    CCV(:,:) = 0.d0
    do s=1,kksl
       if (kstyp(s) > 0) then
          c1 = kksc1(s)
          c2 = kksc2(s)

          hssv  = 0.5d0 * ssv(kk1v,s)

          ccv(kk1v,c1) = ccv(kk1v,c1) + hssv
          ccv(kk1v,c2) = ccv(kk1v,c2) + hssv
       endif
    enddo      !s
  */
}

void gradientZatP(Mesh const &m, Entity::Subset const &s,
    std::vector<double> const &d, std::vector<PtCoord> grad) {

  // Need: var, ccv, csurf, surz, pnorm
  /*
   Z_ForceScatter(dd, KZ_VALID)
   For each point p
     For each active corner c on that point
       point_vol(p) += ccv(kk1a, c)  ! sum the control volume volume
       point_grad(p) = point_grad(p) + var(z(c)) * csurf(c)
     If p is active and valid:
       for each s on p:
         If kksz(s) and kksz2(s) are valid:
           point_grad(p) += var(kksz(s)) * surz(s)
    P_GathScat pvol and pgrad
    ! Normmalize point_grad by point_vol
    for each point p that is active and valid
       point_grad(p) /= point_vol(p)
       if kptyp(p) < 0: ! boundary
         point_grad(p) -= dot(point_grad(p) * pnorm(p)) * pnorm(p)
  */
}

void gradientZatZ(Mesh const &m, Entity::Subset const &s,
    std::vector<double> const &d, std::vector<PtCoord> grad) {}

} // namespace SOA_Idx
} // namespace Ume
