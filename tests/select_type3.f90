module class_default_select_type
    implicit none
    public :: toml_table, get_table
    type :: toml_value
        integer :: int
        real :: float
    end type

    type, extends(toml_value) :: toml_table
      logical :: implicit = .false.
      logical :: inline = .false.
    end type

    type :: enum_stat
      integer :: success = 0
      integer :: fatal = -1
   end type enum_stat

   type(enum_stat), parameter :: toml_stat = enum_stat()

    contains
    subroutine get_table(table, ptr, stat)

        class(toml_table), intent(inout) :: table
        type(toml_table), pointer, intent(out) :: ptr

        integer, intent(out), optional :: stat

        class(toml_value), pointer :: tmp

        nullify(ptr)
        if (associated(tmp)) then
           select type(tmp)
           type is(toml_table)
              ptr => tmp
              if (present(stat)) stat = toml_stat%success
           class default
              if (present(stat)) stat = toml_stat%fatal
           end select
        else
           call check_table(table)
        end if

     end subroutine get_table

    subroutine check_table(tab)
        class(toml_table), intent(out) :: tab
    end subroutine check_table

 end module class_default_select_type
