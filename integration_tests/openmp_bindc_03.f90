module bindc_03_thread_data_module
    use, intrinsic :: iso_c_binding
    type, bind(C) :: thread_data
        integer(c_int) :: n
        type(c_ptr) :: a
    end type thread_data
end module bindc_03_thread_data_module

module module_openmp_bindc_03
use iso_c_binding
implicit none

interface
subroutine GOMP_parallel (fn, data, num_threads, flags) bind (C, name="GOMP_parallel")
import :: c_funptr, c_ptr, c_int
type(c_funptr), value :: fn
type(c_ptr), value :: data
integer(c_int), value :: num_threads
integer(c_int), value :: flags
end subroutine

subroutine GOMP_barrier() bind(C, name="GOMP_barrier")
end subroutine

subroutine GOMP_critical_start() bind(C, name="GOMP_critical_start")
end subroutine

subroutine GOMP_critical_end() bind(C, name="GOMP_critical_end")
end subroutine

function omp_get_max_threads() bind(c, name="omp_get_max_threads")
import :: c_int
integer(c_int) :: omp_get_max_threads
end function omp_get_max_threads

function omp_get_thread_num() bind(c, name="omp_get_thread_num")
import :: c_int
integer(c_int) :: omp_get_thread_num
end function omp_get_thread_num

subroutine omp_set_num_threads(n) bind(c, name="omp_set_num_threads")
import :: c_int
integer(c_int), value :: n
end subroutine omp_set_num_threads

subroutine GOMP_atomic_start() bind(C, name="GOMP_atomic_start")
end subroutine

subroutine GOMP_atomic_end() bind(C, name="GOMP_atomic_end")
end subroutine

end interface

end module

subroutine lcompilers_initialise_array(data) bind(C)
use bindc_03_thread_data_module
use iso_c_binding
use module_openmp_bindc_03
implicit none
type(c_ptr), value :: data
type(thread_data), pointer :: tdata
real(c_float), pointer :: a(:)

integer(c_int) :: i, n, num_threads, chunk, leftovers, thread_num, start, end

call c_f_pointer(data, tdata)

n = tdata%n

call c_f_pointer(tdata%a, a, [n])

num_threads = omp_get_max_threads()
chunk = n / num_threads
leftovers = mod(n, num_threads)

thread_num = omp_get_thread_num()
start = chunk * thread_num

if (thread_num < leftovers) then
    start = start + thread_num
else
    start = start + leftovers
end if

end = start + chunk

if (thread_num < leftovers) then
    end = end + 1
end if

do i = start + 1, end
    ! print *, "Thread ", thread_num, " is processing element ", i
    a(i) = 12.91
end do

call GOMP_barrier()

end subroutine

subroutine initialize_array(n, a)
use bindc_03_thread_data_module
use module_openmp_bindc_03
implicit none

interface
subroutine lcompilers_initialise_array(data) bind(C)
use iso_c_binding
type(c_ptr), value :: data
end subroutine
end interface

integer(c_int), intent(in) :: n
real(c_float), dimension(:), intent(inout), pointer :: a

type(thread_data), target :: data
type(c_ptr) :: tdata

data%n = n
allocate(a(data%n))
data%a = c_loc(a)

tdata = c_loc(data)

call GOMP_parallel(c_funloc(lcompilers_initialise_array), tdata, 0, 0)

end subroutine

program openmp_bindc_03
use omp_lib
use module_openmp_bindc_03
use bindc_03_thread_data_module
implicit none

interface
subroutine initialize_array(n, a)
use iso_c_binding
integer(c_int), intent(in) :: n
real(c_float), intent(inout), dimension(:), pointer :: a
end subroutine
end interface

integer(c_int) :: n = 1000000
real(c_float), dimension(:), pointer :: a

call omp_set_num_threads(4)
call initialize_array(n, a)
print *, "Done"

print *, "a[1] = ", a(1)
print * , "a[1000000] = ", a(1000000)
if (abs(a(1) - 12.91) > 1e-8) error stop
if (abs(a(1000000) - 12.91) > 1e-8) error stop
end program
