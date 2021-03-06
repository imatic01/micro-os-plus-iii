/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2015 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CMSIS_PLUS_POSIX_IO_IO_H_
#define CMSIS_PLUS_POSIX_IO_IO_H_

#if defined(__cplusplus)

// ----------------------------------------------------------------------------

#include <cmsis-plus/posix-io/types.h>

#include <cstddef>
#include <cstdarg>

// Needed for ssize_t
#include <sys/types.h>

// ----------------------------------------------------------------------------

struct iovec;

namespace os
{
  namespace posix
  {
    // ------------------------------------------------------------------------

    class io;
    class file_system;

    /**
     * @ingroup cmsis-plus-posix-io-func
     * @{
     */

    // ------------------------------------------------------------------------
    io*
    open (const char* path, int oflag, ...);

    io*
    vopen (const char* path, int oflag, std::va_list args);

    /**
     * @}
     */

    // ------------------------------------------------------------------------
    /**
     * @brief Base I/O class.
     * @headerfile io.h <cmsis-plus/posix-io/io.h>
     * @ingroup cmsis-plus-posix-io-base
     */
    class io
    {
      // ----------------------------------------------------------------------

      /**
       * @cond ignore
       */

      friend class file_system;
      friend class file_descriptors_manager;

      friend io*
      vopen (const char* path, int oflag, std::va_list args);

      /**
       * @endcond
       */

      // ----------------------------------------------------------------------
    public:

      /**
       * @name Types & Constants
       * @{
       */

      using type_t = unsigned int;
      enum type
        : type_t
          { unknown = 0,
        not_set = 1 << 0,
        device = 1 << 1,
        file = 1 << 2,
        socket = 1 << 3
      };

      /**
       * @}
       */

      // ----------------------------------------------------------------------
      /**
       * @name Constructors & Destructor
       * @{
       */

    protected:

      io (type t);

      /**
       * @cond ignore
       */

      // The rule of five.
      io (const io&) = delete;
      io (io&&) = delete;
      io&
      operator= (const io&) = delete;
      io&
      operator= (io&&) = delete;

      /**
       * @endcond
       */

    public:
      virtual
      ~io ();

      /**
       * @}
       */

      // ----------------------------------------------------------------------
      /**
       * @name Public Member Functions
       * @{
       */

    public:

      int
      close (void);

      ssize_t
      read (void* buf, std::size_t nbyte);

      ssize_t
      write (const void* buf, std::size_t nbyte);

      ssize_t
      writev (const struct iovec* iov, int iovcnt);

      int
      fcntl (int cmd, ...);

      int
      vfcntl (int cmd, std::va_list args);

      int
      isatty (void);

      int
      fstat (struct stat* buf);

      // ----------------------------------------------------------------------
      // Support functions.

      type
      get_type (void) const;

      file_descriptor_t
      file_descriptor (void) const;

#if 0
      bool
      is_opened (void);

      bool
      is_connected (void);
#endif

      /**
       * @}
       */

      // ----------------------------------------------------------------------
      /**
       * @name Private Member Functions
       * @{
       */

    protected:

      // Implementations.

      // do_vopen() is not here, because it is not common
      // (for example for sockets()).

      virtual int
      do_close (void);

      virtual ssize_t
      do_read (void* buf, std::size_t nbyte);

      virtual ssize_t
      do_write (const void* buf, std::size_t nbyte);

      virtual ssize_t
      do_writev (const struct iovec* iov, int iovcnt);

      virtual int
      do_vfcntl (int cmd, std::va_list args);

      virtual int
      do_isatty (void);

      virtual int
      do_fstat (struct stat* buf);

      // ----------------------------------------------------------------------
      // Support functions.

      // Is called at the end of close, to release objects
      // acquired from a pool.
      virtual void
      do_release (void);

      virtual bool
      do_is_opened (void);

      virtual bool
      do_is_connected (void);

      void
      file_descriptor (file_descriptor_t fildes);

      void
      clear_file_descriptor (void);

      io*
      alloc_file_descriptor (void);

      /**
       * @}
       */

      // ----------------------------------------------------------------------
    private:

      /**
       * @cond ignore
       */

      type type_ = type::not_set;

      file_descriptor_t file_descriptor_ = no_file_descriptor;

      /**
       * @endcond
       */

    };

  } /* namespace posix */
} /* namespace os */

// ===== Inline & template implementations ====================================

namespace os
{
  namespace posix
  {
    // ------------------------------------------------------------------------

    inline io::type
    io::get_type (void) const
    {
      return type_;
    }

    inline void
    io::file_descriptor (file_descriptor_t fildes)
    {
      file_descriptor_ = fildes;
    }

    inline void
    io::clear_file_descriptor (void)
    {
      file_descriptor_ = no_file_descriptor;
    }

    inline file_descriptor_t
    io::file_descriptor (void) const
    {
      return file_descriptor_;
    }

#if 0
    inline bool
    io::is_opened (void)
      {
        return do_is_opened ();
      }

    inline bool
    io::is_connected (void)
      {
        return do_is_connected ();
      }
#endif
    ;
  // Avoid formatter bug
  } /* namespace posix */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_POSIX_IO_IO_H_ */
