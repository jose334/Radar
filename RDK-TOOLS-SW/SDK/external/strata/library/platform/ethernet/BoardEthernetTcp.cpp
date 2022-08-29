/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardEthernetTcp.hpp"
#include "BridgeEthernetTcp.hpp"

#include <common/Logger.hpp>
#include <platform/BoardDescriptor.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/templates/searchBoardFunction.hpp>


std::unique_ptr<BoardDescriptor> BoardEthernetTcp::searchBoard(ipAddress_t ipAddr, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    LOG(DEBUG) << "Looking for board at " << std::dec
               << static_cast<int>(ipAddr[0]) << "."
               << static_cast<int>(ipAddr[1]) << "."
               << static_cast<int>(ipAddr[2]) << "."
               << static_cast<int>(ipAddr[3]) << " over TCP ...";

    return searchBoardFunctionBridge<BridgeEthernetTcp>(ipAddr, begin, end);
}

std::unique_ptr<BoardInstance> BoardEthernetTcp::createBoardInstance(ipAddress_t ipAddr)
{
    return searchBoard(ipAddr, BoardListProtocol::begin, BoardListProtocol::end)->createBoardInstance();
}
