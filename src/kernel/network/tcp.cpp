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
