.. _config_network_filters_kafka_mesh:

Kafka Mesh filter
===================

The Apache Kafka mesh filter provides a facade for `Apache Kafka <https://kafka.apache.org/>`_
producers. Produce requests sent to this filter insance can be forwarded to one of multiple
clusters, depending on configured forwarding rules. Corresponding message versions from
Kafka 2.8.1 are supported.

* :ref:`v3 API reference <envoy_v3_api_msg_extensions.filters.network.kafka_mesh.v3alpha.KafkaMesh>`
* This filter should be configured with the name *envoy.filters.network.kafka_mesh*.

.. attention::

   The Kafka mesh filter is only included in :ref:`contrib images <install_contrib>`

.. attention::

   The kafka_mesh filter is experimental and is currently under active development.
   Capabilities will be expanded over time and the configuration structures are likely to change.

.. attention::

   The kafka_mesh filter is does not work on Windows (the blocker is getting librdkafka compiled).

.. _config_network_filters_kafka_mesh_config:

Configuration
-------------

Below example shows us typical filter configuration that proxies 3 Kafka clusters.
Clients are going to connect to '127.0.0.1:19092', and their messages are going to be distributed
to cluster depending on topic names.

.. code-block:: yaml

  listeners:
  - address:
      socket_address:
        address: 127.0.0.1 # Host that Kafka clients should connect to.
        port_value: 19092  # Port that Kafka clients should connect to.
    filter_chains:
    - filters:
      - name: envoy.filters.network.kafka_mesh
        typed_config:
          "@type": type.googleapis.com/envoy.extensions.filters.network.kafka_mesh.v3alpha.KafkaMesh
          advertised_host: "127.0.0.1"
          advertised_port: 19092
          upstream_clusters:
          - cluster_name: kafka_c1
            bootstrap_servers: cluster1_node1:9092,cluster1_node2:9092,cluster1_node3:9092
            partition_count: 1
          - cluster_name: kafka_c2
            bootstrap_servers: cluster2_node1:9092,cluster2_node2:9092,cluster2_node3:9092
            partition_count: 1
          - cluster_name: kafka_c3
            bootstrap_servers: cluster3_node1:9092,cluster3_node2:9092
            partition_count: 5
            producer_config:
              acks: "1"
              linger.ms: "500"
          forwarding_rules:
          - target_cluster: kafka_c1
            topic_prefix: apples
          - target_cluster: kafka_c2
            topic_prefix: bananas
          - target_cluster: kafka_c3
            topic_prefix: cherries

It should be noted that Kafka broker filter can be inserted before Kafka mesh filter in the filter
chain to capture the request processing metrics.

.. _config_network_filters_kafka_mesh_notes:

Notes
-----
Given that this filter does its own processing of received requests, there are some changes
in behaviour compared to explicit connection to a Kafka cluster:

#. Record headers are not sent upstream.
#. Only ProduceRequests with version 2 are supported (what means very old producers like 0.8 are
   not going to be supported).
#. Python producers need to set API version of at least 1.0.0, so that the produce requests they
   send are going to have records with magic equal to 2.
#. Downstream handling of Kafka producer 'acks' property is delegated to upstream client.
   E.g. if upstream client is configured to use acks=0 then the response is going to be sent
   to downstream client as soon as possible (even if they had non-zero acks!).
#. As the filter splits single producer requests into separate records, it's possible that delivery
   of only some of these records fails. In that case, the response returned to upstream client is
   a failure, however it is possible some of the records have been appended in target cluster.
#. Because of the splitting mentioned above, records are not necessarily appended one after another
   (as they do not get sent as single request to upstream). Users that want to avoid this scenario
   might want to take a look into downstream producer configs: 'linger.ms' and 'batch.size'.
#. Produce requests that reference to topics that do not match any of the rules are going to close
   connection and fail. This usually should not happen (clients request metadata first, and they
   should then fail with 'no broker available' first), but is possible if someone tailors binary
   payloads over the connection.
#. librdkafka was compiled without ssl, lz4, gssapi, so related custom producer config options are
   not supported.
#. Invalid custom producer configs are not found at startup (only when appropriate clusters are
   being sent to). Requests that would have referenced these clusters are going to close connection
   and fail.
