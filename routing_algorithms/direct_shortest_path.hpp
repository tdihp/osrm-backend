/*

Copyright (c) 2015, Project OSRM contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef DIRECT_SHORTEST_PATH_HPP
#define DIRECT_SHORTEST_PATH_HPP

#include <boost/assert.hpp>

#include "routing_base.hpp"
#include "../data_structures/search_engine_data.hpp"
#include "../util/integer_range.hpp"
#include "../util/timing_util.hpp"
#include "../typedefs.h"

template <class DataFacadeT>
class DirectShortestPathRouting final	//dirty hack to implement direct routing without via nodes more efficiently
    : public BasicRoutingInterface<DataFacadeT, DirectShortestPathRouting<DataFacadeT>>
{
    using super = BasicRoutingInterface<DataFacadeT, DirectShortestPathRouting<DataFacadeT>>;
    using QueryHeap = SearchEngineData::QueryHeap;
    SearchEngineData &engine_working_data;

  public:
    DirectShortestPathRouting(DataFacadeT *facade, SearchEngineData &engine_working_data)
        : super(facade), engine_working_data(engine_working_data)
    {
    }

    ~DirectShortestPathRouting() {}

    void operator()(const std::vector<PhantomNodes> &phantom_nodes_vector,
                    const std::vector<bool> &uturn_indicators,
                    InternalRouteResult &raw_route_data) const
    {
        int distance1 = 0;
        NodeID middle1 = SPECIAL_NODEID;
        std::vector<std::vector<NodeID>> packed_legs1(phantom_nodes_vector.size());

        engine_working_data.InitializeOrClearFirstThreadLocalStorage(
            super::facade->GetNumberOfNodes());
        engine_working_data.InitializeOrClearSecondThreadLocalStorage(
            super::facade->GetNumberOfNodes());
        engine_working_data.InitializeOrClearThirdThreadLocalStorage(
            super::facade->GetNumberOfNodes());

        QueryHeap &forward_heap1 = *(engine_working_data.forward_heap_1);
        QueryHeap &reverse_heap1 = *(engine_working_data.reverse_heap_1);

        std::size_t current_leg = 0;
        // Get distance to next pair of target nodes.
		BOOST_ASSERT_MSG(1==phantom_nodes_vector.size(),
						 "Direct Shortest Path Query only accepts a single source and target pair. Multiple ones have been specified.");
		const PhantomNodes &phantom_node_pair = phantom_nodes_vector[0];
        {
			//TIMER_START( t_init );
            forward_heap1.Clear();
            reverse_heap1.Clear();
            int local_upper_bound1 = INVALID_EDGE_WEIGHT;

            middle1 = SPECIAL_NODEID;

            const EdgeWeight min_edge_offset =
                std::min(-phantom_node_pair.source_phantom.GetForwardWeightPlusOffset(),
                         -phantom_node_pair.source_phantom.GetReverseWeightPlusOffset());

            // insert new starting nodes into forward heap, adjusted by previous distances.
            if (phantom_node_pair.source_phantom.forward_node_id != SPECIAL_NODEID)
            {
                forward_heap1.Insert(
                    phantom_node_pair.source_phantom.forward_node_id,
                    -phantom_node_pair.source_phantom.GetForwardWeightPlusOffset(),
                    phantom_node_pair.source_phantom.forward_node_id);
                // SimpleLogger().Write(logDEBUG) << "fwd-a2 insert: " <<
                // phantom_node_pair.source_phantom.forward_node_id << ", w: " << -phantom_node_pair.source_phantom.GetForwardWeightPlusOffset();
            }
            if ( phantom_node_pair.source_phantom.reverse_node_id != SPECIAL_NODEID)
            {
                forward_heap1.Insert(
                    phantom_node_pair.source_phantom.reverse_node_id,
                    -phantom_node_pair.source_phantom.GetReverseWeightPlusOffset(),
                    phantom_node_pair.source_phantom.reverse_node_id);
                // SimpleLogger().Write(logDEBUG) << "fwd-a2 insert: " <<
                // phantom_node_pair.source_phantom.reverse_node_id << ", w: " << -phantom_node_pair.source_phantom.GetReverseWeightPlusOffset();
            }

            // insert new backward nodes into backward heap, unadjusted.
            if (phantom_node_pair.target_phantom.forward_node_id != SPECIAL_NODEID)
            {
                reverse_heap1.Insert(phantom_node_pair.target_phantom.forward_node_id,
                                     phantom_node_pair.target_phantom.GetForwardWeightPlusOffset(),
                                     phantom_node_pair.target_phantom.forward_node_id);
                // SimpleLogger().Write(logDEBUG) << "rev-a insert: " <<
                // phantom_node_pair.target_phantom.forward_node_id << ", w: " << phantom_node_pair.target_phantom.GetForwardWeightPlusOffset();
            }

            if (phantom_node_pair.target_phantom.reverse_node_id != SPECIAL_NODEID)
            {
                reverse_heap1.Insert(phantom_node_pair.target_phantom.reverse_node_id,
                                     phantom_node_pair.target_phantom.GetReverseWeightPlusOffset(),
                                     phantom_node_pair.target_phantom.reverse_node_id);
                // SimpleLogger().Write(logDEBUG) << "rev-a insert: " <<
                // phantom_node_pair.target_phantom.reverse_node_id << ", w: " << phantom_node_pair.target_phantom.GetReverseWeightPlusOffset();
            }
			//TIMER_STOP( t_init );
			//std::cout << "[init] " << TIMER_MSEC( t_init ) << std::endl;

			//ch stopping criterion
			int local_minimum_forward = 0;
			int local_minimum_reverse = 0;

			//TIMER_START( t_query_1 );
            // run two-Target Dijkstra routing step.
            while (0 < (forward_heap1.Size() + reverse_heap1.Size()) && local_upper_bound1 > local_minimum_forward + local_minimum_reverse )
            //while (0 < (forward_heap1.Size() + reverse_heap1.Size()) )
			{
                if (!forward_heap1.Empty())
                {
                    super::RoutingStep(forward_heap1, reverse_heap1, &middle1, &local_upper_bound1,
                                       min_edge_offset, true);
					if(!forward_heap1.Empty()){
						local_minimum_forward = forward_heap1.Min();
					}
                }
                if (!reverse_heap1.Empty())
                {
                    super::RoutingStep(reverse_heap1, forward_heap1, &middle1, &local_upper_bound1,
                                       min_edge_offset, false);
					if(!reverse_heap1.Empty()){
						local_minimum_reverse = reverse_heap1.Min();
					}
                }
            }
			//TIMER_STOP( t_query_1 );
			//std::cout << "[q] " << TIMER_MSEC( t_query_1 ) << std::endl;

			//TIMER_START( t_postproc );
            // No path found for both target nodes?
            if ((INVALID_EDGE_WEIGHT == local_upper_bound1))
            {
                raw_route_data.shortest_path_length = INVALID_EDGE_WEIGHT;
                raw_route_data.alternative_path_length = INVALID_EDGE_WEIGHT;
                return;
            }

            // Was a paths over one of the forward/reverse nodes not found?
            BOOST_ASSERT_MSG((SPECIAL_NODEID == middle1 || INVALID_EDGE_WEIGHT != distance1),
                             "no path found");

            // Unpack paths if they exist
            std::vector<NodeID> temporary_packed_leg1;

            BOOST_ASSERT(current_leg < packed_legs1.size());

            if (INVALID_EDGE_WEIGHT != local_upper_bound1)
            {
                super::RetrievePackedPathFromHeap(forward_heap1, reverse_heap1, middle1,
                                                  temporary_packed_leg1);
            }

            BOOST_ASSERT_MSG(!temporary_packed_leg1.empty(),
                             "tempory packed path empty");

            BOOST_ASSERT((0 == current_leg));

            std::swap( packed_legs1[current_leg], temporary_packed_leg1 );
            BOOST_ASSERT(packed_legs1[current_leg].size() && 0==temporary_packed_leg1.size());

            distance1 = local_upper_bound1;
			//TIMER_STOP( t_postproc );
			//std::cout << "[pp] " << TIMER_MSEC( t_postproc ) << std::endl;
			
			//std::cout << "[info] dist: " << distance1 << " segsize: " << temporary_packed_leg1.size() << std::endl;
        }



        raw_route_data.unpacked_path_segments.resize(packed_legs1.size());

        for (const std::size_t index : osrm::irange<std::size_t>(0, packed_legs1.size()))
        {
            BOOST_ASSERT(!phantom_nodes_vector.empty());
            BOOST_ASSERT(packed_legs1.size() == raw_route_data.unpacked_path_segments.size());

            PhantomNodes unpack_phantom_node_pair = phantom_nodes_vector[index];
            super::UnpackPath(
                // -- packed input
                packed_legs1[index],
                // -- start and end of (sub-)route
                unpack_phantom_node_pair,
                // -- unpacked output
                raw_route_data.unpacked_path_segments[index]);

            raw_route_data.source_traversed_in_reverse.push_back(
                (packed_legs1[index].front() !=
                 phantom_nodes_vector[index].source_phantom.forward_node_id));
            raw_route_data.target_traversed_in_reverse.push_back(
                (packed_legs1[index].back() !=
                 phantom_nodes_vector[index].target_phantom.forward_node_id));
        }
        raw_route_data.shortest_path_length = distance1;
    }
};

#endif /* DIRECT_SHORTEST_PATH_HPP */
