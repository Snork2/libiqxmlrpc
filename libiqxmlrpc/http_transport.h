//  Libiqnet + Libiqxmlrpc - an object-oriented XML-RPC solution.
//  Copyright (C) 2004 Anton Dedov
//  
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//  
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//  
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
//  
//  $Id: http_transport.h,v 1.10 2004-04-22 09:25:56 adedov Exp $

#ifndef _libiqxmlrpc_http_transport_h_
#define _libiqxmlrpc_http_transport_h_

#include "connector.h"
#include "server.h"
#include "client.h"

namespace iqxmlrpc
{
  class Http_server_connection;
  class Http_client_connection;
};


//! Represents server-side \b HTTP non-blocking connection.
class iqxmlrpc::Http_server_connection: public iqxmlrpc::Server_connection {
  iqnet::Reactor* reactor;

public:
  Http_server_connection( int, const iqnet::Inet_addr& );

  void set_reactor( iqnet::Reactor* r ) { reactor = r; }

  void post_accept();  
  void finish();
  
  void handle_input( bool& );
  void handle_output( bool& );

  void schedule_response( http::Packet* packet );
};


//! XML-RPC \b HTTP client's connection (works in blocking mode).
class iqxmlrpc::Http_client_connection: public iqxmlrpc::Client_connection {
public:
  Http_client_connection( int sock, const iqnet::Inet_addr& peer ):
    Client_connection( sock, peer ) {}

protected:
  http::Packet* do_process_session( const std::string& );
};


#endif
