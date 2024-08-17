#include "network/tcp.h"

namespace Networking
{
    namespace TransmissionControlProtocol
    {

        uint16_t bigEndian16(uint16_t x)
        {
            return (x & 0xFF00) >> 8 | (x&0x00FF) << 8;
        }

        uint32_t bigEndian32(uint32_t x)
        {
            return (x & 0xFF000000) >> 24
                | (x & 0x00FF0000) >> 8
                | (x & 0x0000FF00) << 8
                | (x & 0x000000FF) << 24;
        }

        void TransmissionControlProtocolSocket::receive_on_listen(TransmissionControlProtocolHeader* header)
        {
            // We have not yet sent anything to be acknowledged so drop the packet if it sets the ACK flag.
            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::ACK) { return; }
            // We need SYN flag to establish a connection.
            if (!(header->get_flags() & TransmissionControlProtocolHeader::Flag::SYN)) { return; }

            // The IRS is the initial receive sequence number.
            irs = bigEndian32(header->get_seq_number());
            rcv_nxt = bigEndian32(header->get_seq_number()) + 1;
            snd_una = bigEndian32(header->get_ack_number());
            snd_wnd = bigEndian32(header->get_window_size());
            snd_wl1 = bigEndian32(header->get_seq_number());
            snd_wl2 = bigEndian32(header->get_ack_number());

            m_state = SYN_RECEIVED;
            send(snd_nxt, 0, 0, TransmissionControlProtocolHeader::Flag::SYN | TransmissionControlProtocolHeader::Flag::ACK);
        }

        void TransmissionControlProtocolSocket::send(uint8_t* data, uint16_t size)
        {
            send(snd_nxt, data, size, TransmissionControlProtocolHeader::Flag::ACK);   
        }

        void TransmissionControlProtocolSocket::receive_on_syn_sent(TransmissionControlProtocolHeader* header)
        {
            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::ACK)
            {
                if (bigEndian32(header->get_ack_number()) <= iss || bigEndian32(header->get_ack_number()) > snd_nxt)
                {
                    // reset?
                    return;
                }
            }

            if (!(header->get_flags() & TransmissionControlProtocolHeader::Flag::SYN)) { return; }

            irs = bigEndian32(header->get_seq_number());
            rcv_nxt = bigEndian32(header->get_seq_number()) + 1;
            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::ACK)
            {
                snd_una = bigEndian32(header->get_ack_number());
                snd_wnd = bigEndian16(header->get_window_size());
                snd_wl1 = bigEndian32(header->get_seq_number());
                snd_wl2 = bigEndian32(header->get_ack_number());
                
                m_state = ESTABLISHED;
                send(snd_nxt, 0, 0, TransmissionControlProtocolHeader::Flag::ACK);
            }
            else
            {
                m_state = SYN_RECEIVED;
                // Our SYN that we sent has not been acknowledged, send it again.
                snd_nxt--;
                send(snd_nxt, 0, 0, TransmissionControlProtocolHeader::Flag::ACK | TransmissionControlProtocolHeader::Flag::SYN);
            }
        }

        void TransmissionControlProtocolSocket::refresh_retransmission_queue()
        {
            for (auto it = m_retransmissionList.first(); !it.is_end(); ++it)
            {
                if (it->sequenceNumber < snd_una)
                {
                    it = m_retransmissionList.remove(it);
                    continue;
                }

                /*if (pit_ticks >= expiry)
                {

                    it = m_retransmissionList.remove(it);
                    // remove and resend
                    send(it->sequenceNumber, (uint8_t*)it->data, it->length, TransmissionControlProtocolHeader::Flag::ACK);
                }*/
            }
        }


        void TransmissionControlProtocolSocket::send(uint32_t seq, uint8_t* data, uint16_t size, uint16_t flags)
        {
            auto sizeInclPseudoHeader = sizeof(TransmissionControlProtocolPseudoHeader) + sizeof(TransmissionControlProtocolHeader) + size;
            auto buf = (uint8_t*)kmalloc(sizeInclPseudoHeader, 0);
            auto pseudoHeader = (TransmissionControlProtocolPseudoHeader*) buf;
            auto header = (TransmissionControlProtocolHeader*)(buf+sizeof(TransmissionControlProtocolPseudoHeader));

            header->set_ack_number(flags & TransmissionControlProtocolHeader::Flag::ACK ? bigEndian32(rcv_nxt) : 0);
            header->set_seq_number(bigEndian32(seq));
            header->set_flags(flags);
            header->set_source_port(m_localPort);
            header->set_dest_port(m_remotePort);
            header->set_window_size(bigEndian16(8192));
            header->set_checksum(0);
            header->set_options(((flags & TransmissionControlProtocolHeader::Flag::SYN) != 0) ? 0xB4050402 : 0);
            header->set_data_offset(sizeof(TransmissionControlProtocolHeader)/4);
            header->set_urgent_pointer(0);
            pseudoHeader->set_dest_ip(m_localIP);
            pseudoHeader->set_source_ip(m_remoteIP);
            pseudoHeader->set_length(bigEndian16(size + sizeof(TransmissionControlProtocolHeader)));
            pseudoHeader->set_protocol(0x0600);

            // Copy the packet into a buffer.
            memcpy(buf+sizeof(TransmissionControlProtocolPseudoHeader)+sizeof(TransmissionControlProtocolHeader), data, size);

            header->set_checksum(InternetProtocolV4::InternetProtocolManager::instance()->checksum((uint16_t*)buf, sizeInclPseudoHeader));

            InternetProtocolV4::InternetProtocolManager::instance()->send_data(m_remoteIP, 0x0006, (uint8_t*)header, size+sizeof(TransmissionControlProtocolHeader));
            snd_nxt += size;
            if (flags & (TransmissionControlProtocolHeader::Flag::FIN | TransmissionControlProtocolHeader::Flag::SYN)) { snd_nxt++; }
            //kfree(buf);
        }
        
        void TransmissionControlProtocolSocket::receive_ack(TransmissionControlProtocolHeader* header)
        {
            switch (m_state)
            {
            case SYN_RECEIVED:
                // Check whether the acknowledgement number is in the send window.
                if (snd_una <= header->get_ack_number() && header->get_ack_number() <= snd_nxt)
                {
                    snd_wnd = header->get_window_size();
                    // SND_WL1 is the sequence number of the packet that last updated the window.
                    snd_wl1 = header->get_seq_number();
                    // SND_WL2 is the ack number of the packet that last updated the window.
                    snd_wl2 = header->get_ack_number();
                    m_state = ESTABLISHED;
                }
                else
                {
                    send(header->get_ack_number(), 0, 0, TransmissionControlProtocolHeader::Flag::RST | TransmissionControlProtocolHeader::Flag::ACK);
                }
                break;
            
            case ESTABLISHED:
                
                if (snd_una <= header->get_ack_number() && header->get_ack_number() <= snd_nxt)
                {
                    // We have now received an ack for the first bytes in the send window.
                    // Therefore, move the send window up.
                    snd_una = header->get_ack_number();

                    // We need to update the window
                    if (snd_wl1 < header->get_seq_number() || (snd_wl1 == header->get_seq_number() && snd_wl2 < header->get_ack_number()))
                    {
                        snd_wnd = header->get_window_size();
                        snd_wl1 = header->get_seq_number();
                        snd_wl2 = header->get_ack_number();
                    }

                    // We store the unacknowledged packets in the retransmission queue in case we need to send them again.
                    // Update the queue to remove the now acknowledged packets.

                    refresh_retransmission_queue();
                }
            }
        }

        void TransmissionControlProtocolSocket::receive_rst(TransmissionControlProtocolHeader* header) {}

        void TransmissionControlProtocolSocket::receive_fin(TransmissionControlProtocolHeader* header)
        {
            // We just acknowledge the FIN.
            rcv_nxt = bigEndian32(header->get_seq_number()) + 1;
            send(snd_nxt, 0, 0, TransmissionControlProtocolHeader::Flag::ACK);

            switch (m_state)
            {
            case SYN_RECEIVED:
            case ESTABLISHED:
                m_state = CLOSE_WAIT;
                break;
            case FIN_WAIT1:
                if (header->get_ack_number() > snd_nxt)
                {
                    m_state = TIME_WAIT;
                }
                else
                {
                    m_state = CLOSING;
                }
                break;
            case FIN_WAIT2:
            case TIME_WAIT:
                m_state = TIME_WAIT;
                break;
            default:
                break;
            }
        }

        void TransmissionControlProtocolSocket::receive_TCP_message(InternetProtocolV4Address source, InternetProtocolV4Address dest, uint8_t* data, uint16_t size)
        {
            // We need to include a pseudoheader to calculate the checksum and verify it.
            auto sizeInclPseudoHeader = sizeof(TransmissionControlProtocolPseudoHeader) + size;
            auto buf = (uint8_t*)kmalloc(sizeInclPseudoHeader, 0);

            memcpy(buf+sizeof(TransmissionControlProtocolPseudoHeader), data, size);
            auto pseudoHeader = (TransmissionControlProtocolPseudoHeader*)buf;
            auto header = (TransmissionControlProtocolHeader*)(buf+sizeof(TransmissionControlProtocolPseudoHeader));
            pseudoHeader->set_dest_ip(dest);
            pseudoHeader->set_source_ip(source);
            pseudoHeader->set_length((bigEndian16(size)));
            pseudoHeader->set_protocol(0x0600);

            // Drop the packet if the checksum is not valid.
            if (InternetProtocolV4::InternetProtocolManager::instance()->checksum((uint16_t*)buf, sizeInclPseudoHeader))
                return;

            header = (TransmissionControlProtocolHeader*)data;
            
            // Handle the cases that we are either listening as a server or trying to connect as a client.
            switch(m_state)
            {
            case LISTEN:
                return receive_on_listen(header);
            case SYN_SENT:
                return receive_on_syn_sent(header);
            default:
                break;
            }

            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::RST)
            {
                return receive_rst(header);
            }

            // We have already handled the case that we are listening or trying to connect, so if we have
            // a SYN flag (used to intialise connections), we need to reset the connection.

            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::SYN)
            {
                return send(0, 0, 0, TransmissionControlProtocolHeader::Flag::RST | TransmissionControlProtocolHeader::Flag::ACK);
            } 

            // All packets after the initial packet must have the ACK flag set.
            if (!(header->get_flags() & TransmissionControlProtocolHeader::Flag::ACK))
            {
                return;
            }

            // Update with the contents of the new packet.
            receive_ack(header);
            receive_data(data, size);

            // The sender has requested to end the connection.
            if (header->get_flags() & TransmissionControlProtocolHeader::Flag::FIN)
            {
                receive_fin(header);
                return;
            }
        }

        void TransmissionControlProtocolSocket::add_to_received(const TCPPacket& packet)
        {
            auto it = m_outOfOrderList.first();
            for (; !it.is_end(); ++it)
            {
                if (packet.sequenceNumber < it->sequenceNumber) { break; }
            }
            m_outOfOrderList.insert_before(packet, it);
        }

        void TransmissionControlProtocolSocket::process_received()
        {
            for (auto it = m_outOfOrderList.first(); !it.is_end();)
            {
                // If we now have missing packets, we can send them to the application.
                if (rcv_nxt != bigEndian32(it->sequenceNumber)) { break; }
                rcv_nxt += it->length;

                char* print = " ";
                for(int i = 0; i < it->length; i++)
                {
                    print[0] = ((uint8_t*)it->data)[i];
                    printf(print);
                }
                it = m_outOfOrderList.remove(it);
            }
        }

        void TransmissionControlProtocolSocket::receive_data(uint8_t* data, uint16_t size)
        {
            auto header = (TransmissionControlProtocolHeader*)data;
            auto headerSize = header->get_data_offset()*4;

            if (headerSize >= size) { return; }

            switch(m_state)
            {
            case ESTABLISHED:
                TCPPacket packet;
                packet.data = data+headerSize;
                packet.length = size-headerSize;
                packet.sequenceNumber = header->get_seq_number();
                // Add to the received queue.
                add_to_received(packet);
                // Check if we can send the received packets to the application layer.
                process_received();
                // Acknowledge the packet.
                send(snd_nxt, 0, 0, TransmissionControlProtocolHeader::Flag::ACK);
                break;
            default:
                break;
            }
        }
