/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
\file Ume/Comm_Transport.hh
*/

#ifndef UME_COMM_TRANSPORT_HH
#define UME_COMM_TRANSPORT_HH 1

#include "Ume/Comm_Buffers.hh"

namespace Ume {
namespace Comm {

//! A virtual base class for defining communication between mesh partitions
/*! The send/receive Buffers associate the data to Entity fields; this is just
    the low-level transport mechanism. While MPI Transport is the first
    specialization of this, it could also be used for IPC, shared memory, or GPU
    data motion. */
class Transport {
public:
  //! Exchange integer field elements
  virtual void exchange(Buffers<DS_Types::INTV_T> const & /*sends*/,
      Buffers<DS_Types::INTV_T> & /*recvs*/) {}
  //! Excange double precision field elements
  virtual void exchange(Buffers<DS_Types::DBLV_T> const & /*sends*/,
      Buffers<DS_Types::DBLV_T> & /*recvs*/) {}
  //! Exchange VEC3 field elements
  virtual void exchange(Buffers<DS_Types::VEC3V_T> const & /*sends*/,
      Buffers<DS_Types::VEC3V_T> & /*recvs*/) {}

  //! Return some sort of identifier for this node in the Transport graph
  virtual int id() const { return -1; }

  //! Shut down the transport
  virtual int stop() = 0;

  //! Provide a virtual dtor for safe inheritance
  virtual ~Transport() = default;

  //! Provide a way for a client to abort with a message
  virtual void abort(char const *const message);
};

//! This is a null transporter: it doesn't do anythin
class Dummy_Transport : public Transport {
public:
  Dummy_Transport();
  int stop() override { return -1; }
};

} // namespace Comm
} // namespace Ume

#endif
