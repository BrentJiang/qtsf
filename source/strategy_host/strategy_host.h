#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "envoy/event/timer.h"
#include "envoy/server/drain_manager.h"
#include "envoy/server/guarddog.h"
#include "envoy/server/instance.h"
#include "envoy/server/process_context.h"
#include "envoy/server/tracer_config.h"
#include "envoy/ssl/context_manager.h"
#include "envoy/stats/stats_macros.h"
#include "envoy/stats/timespan.h"
#include "envoy/tracing/http_tracer.h"

#include "common/access_log/access_log_manager_impl.h"
#include "common/common/assert.h"
#include "common/common/cleanup.h"
#include "common/common/logger_delegates.h"
#include "common/grpc/async_client_manager_impl.h"
#include "common/grpc/context_impl.h"
#include "common/http/context_impl.h"
#include "common/init/manager_impl.h"
#include "common/memory/heap_shrinker.h"
#include "common/protobuf/message_validator_impl.h"
#include "common/runtime/runtime_impl.h"
#include "common/secret/secret_manager_impl.h"
#include "common/upstream/health_discovery_service.h"

#include "server/configuration_impl.h"
#include "server/http/admin.h"
#include "server/listener_hooks.h"
#include "server/listener_manager_impl.h"
#include "server/overload_manager_impl.h"
#include "server/worker_impl.h"
#include "server/server.h"

#include "absl/container/node_hash_map.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace StrategyHost {

/**
 * This is the actual full standalone server which stitches together various common components.
 */
class StrategyHostInstanceImpl : Logger::Loggable<Logger::Id::main>,
                     public Server::Instance,
                     public Server::ServerLifecycleNotifier {
public:
  /**
   * @throw EnvoyException if initialization fails.
   */
  StrategyHostInstanceImpl(const Server::Options& options, Event::TimeSystem& time_system,
               Network::Address::InstanceConstSharedPtr local_address, ListenerHooks& hooks,
               Server::HotRestart& restarter, Stats::StoreRoot& store,
               Thread::BasicLockable& access_log_lock, Server::ComponentFactory& component_factory,
               Runtime::RandomGeneratorPtr&& random_generator, ThreadLocal::Instance& tls,
               Thread::ThreadFactory& thread_factory, Filesystem::Instance& file_system,
               std::unique_ptr<ProcessContext> process_context);

  ~StrategyHostInstanceImpl() override;

  void run();

  // Server::Instance
  Server::Admin& admin() override { return *admin_; }
  Api::Api& api() override { return *api_; }
  Upstream::ClusterManager& clusterManager() override;
  Ssl::ContextManager& sslContextManager() override { return *ssl_context_manager_; }
  Event::Dispatcher& dispatcher() override { return *dispatcher_; }
  Network::DnsResolverSharedPtr dnsResolver() override { return dns_resolver_; }
  void drainListeners() override;
  Server::DrainManager& drainManager() override { return *drain_manager_; }
  AccessLog::AccessLogManager& accessLogManager() override { return access_log_manager_; }
  void failHealthcheck(bool fail) override;
  Server::HotRestart& hotRestart() override { return restarter_; }
  Init::Manager& initManager() override { return init_manager_; }
  ServerLifecycleNotifier& lifecycleNotifier() override { return *this; }
  Server::ListenerManager& listenerManager() override { return *listener_manager_; }
  Secret::SecretManager& secretManager() override { return *secret_manager_; }
  Envoy::MutexTracer* mutexTracer() override { return mutex_tracer_; }
  Server::OverloadManager& overloadManager() override { return *overload_manager_; }
  Runtime::RandomGenerator& random() override { return *random_generator_; }
  Runtime::Loader& runtime() override;
  void shutdown() override;
  bool isShutdown() final { return shutdown_; }
  void shutdownAdmin() override;
  Singleton::Manager& singletonManager() override { return *singleton_manager_; }
  bool healthCheckFailed() override;
  const Server::Options& options() override { return options_; }
  time_t startTimeCurrentEpoch() override { return start_time_; }
  time_t startTimeFirstEpoch() override { return original_start_time_; }
  Stats::Store& stats() override { return stats_store_; }
  Grpc::Context& grpcContext() override { return grpc_context_; }
  Http::Context& httpContext() override { return http_context_; }
  ProcessContext& processContext() override { return *process_context_; }
  ThreadLocal::Instance& threadLocal() override { return thread_local_; }
  const LocalInfo::LocalInfo& localInfo() override { return *local_info_; }
  TimeSource& timeSource() override { return time_source_; }

  std::chrono::milliseconds statsFlushInterval() const override {
    return config_.statsFlushInterval();
  }

  ProtobufMessage::ValidationContext& messageValidationContext() override {
    return validation_context_;
  }

  // ServerLifecycleNotifier
  ServerLifecycleNotifier::HandlePtr registerCallback(Stage stage, StageCallback callback) override;
  ServerLifecycleNotifier::HandlePtr
  registerCallback(Stage stage, StageCallbackWithCompletion callback) override;

private:
  ProtobufTypes::MessagePtr dumpBootstrapConfig();
  void flushStats();
  void flushStatsInternal();
  void initialize(const Server::Options& options, Network::Address::InstanceConstSharedPtr local_address,
                  Server::ComponentFactory& component_factory, ListenerHooks& hooks);
  void loadServerFlags(const absl::optional<std::string>& flags_path);
  void startWorkers();
  void terminate();
  void notifyCallbacksForStage(
      Stage stage, Event::PostCb completion_cb = [] {});

  // init_manager_ must come before any member that participates in initialization, and destructed
  // only after referencing members are gone, since initialization continuation can potentially
  // occur at any point during member lifetime. This init manager is populated with LdsApi targets.
  Init::ManagerImpl init_manager_{"Server"};
  // secret_manager_ must come before listener_manager_, config_ and dispatcher_, and destructed
  // only after these members can no longer reference it, since:
  // - There may be active filter chains referencing it in listener_manager_.
  // - There may be active clusters referencing it in config_.cluster_manager_.
  // - There may be active connections referencing it.
  std::unique_ptr<Secret::SecretManager> secret_manager_;
  bool workers_started_;
  bool shutdown_;
  const Server::Options& options_;
  ProtobufMessage::ProdValidationContextImpl validation_context_;
  TimeSource& time_source_;
  Server::HotRestart& restarter_;
  const time_t start_time_;
  time_t original_start_time_;
  Stats::StoreRoot& stats_store_;
  std::unique_ptr<Server::ServerStats> server_stats_;
  Assert::ActionRegistrationPtr assert_action_registration_;
  ThreadLocal::Instance& thread_local_;
  Api::ApiPtr api_;
  Event::DispatcherPtr dispatcher_;
  std::unique_ptr<Server::AdminImpl> admin_;
  Singleton::ManagerPtr singleton_manager_;
  Network::ConnectionHandlerPtr handler_;
  Runtime::RandomGeneratorPtr random_generator_;
  std::unique_ptr<Runtime::ScopedLoaderSingleton> runtime_singleton_;
  std::unique_ptr<Ssl::ContextManager> ssl_context_manager_;
  Server::ProdListenerComponentFactory listener_component_factory_;
  Server::ProdWorkerFactory worker_factory_;
  std::unique_ptr<Server::ListenerManager> listener_manager_;
  Server::Configuration::MainImpl config_;
  Network::DnsResolverSharedPtr dns_resolver_;
  Event::TimerPtr stat_flush_timer_;
  LocalInfo::LocalInfoPtr local_info_;
  Server::DrainManagerPtr drain_manager_;
  AccessLog::AccessLogManagerImpl access_log_manager_;
  std::unique_ptr<Upstream::ClusterManagerFactory> cluster_manager_factory_;
  std::unique_ptr<Server::GuardDog> guard_dog_;
  bool terminated_;
  std::unique_ptr<Logger::FileSinkDelegate> file_logger_;
  envoy::config::bootstrap::v2::Bootstrap bootstrap_;
  Server::ConfigTracker::EntryOwnerPtr config_tracker_entry_;
  SystemTime bootstrap_config_update_time_;
  Grpc::AsyncClientManagerPtr async_client_manager_;
  Upstream::ProdClusterInfoFactory info_factory_;
  Upstream::HdsDelegatePtr hds_delegate_;
  std::unique_ptr<Server::OverloadManagerImpl> overload_manager_;
  Envoy::MutexTracer* mutex_tracer_;
  Grpc::ContextImpl grpc_context_;
  Http::ContextImpl http_context_;
  std::unique_ptr<ProcessContext> process_context_;
  std::unique_ptr<Memory::HeapShrinker> heap_shrinker_;
  const std::thread::id main_thread_id_;
  // initialization_time is a histogram for tracking the initialization time across hot restarts
  // whenever we have support for histogram merge across hot restarts.
  Stats::TimespanPtr initialization_timer_;

  using LifecycleNotifierCallbacks = std::list<StageCallback>;
  using LifecycleNotifierCompletionCallbacks = std::list<StageCallbackWithCompletion>;

  template <class T>
  class LifecycleCallbackHandle : public ServerLifecycleNotifier::Handle, RaiiListElement<T> {
  public:
    LifecycleCallbackHandle(std::list<T>& callbacks, T& callback)
        : RaiiListElement<T>(callbacks, callback) {}
  };

  absl::node_hash_map<Stage, LifecycleNotifierCallbacks> stage_callbacks_;
  absl::node_hash_map<Stage, LifecycleNotifierCompletionCallbacks> stage_completable_callbacks_;
};

// Local implementation of Stats::MetricSnapshot used to flush metrics to sinks. We could
// potentially have a single class instance held in a static and have a clear() method to avoid some
// vector constructions and reservations, but I'm not sure it's worth the extra complexity until it
// shows up in perf traces.
// TODO(mattklein123): One thing we probably want to do is switch from returning vectors of metrics
//                     to a lambda based callback iteration API. This would require less vector
//                     copying and probably be a cleaner API in general.
class MetricSnapshotImpl : public Stats::MetricSnapshot {
public:
  explicit MetricSnapshotImpl(Stats::Store& store);

  // Stats::MetricSnapshot
  const std::vector<CounterSnapshot>& counters() override { return counters_; }
  const std::vector<std::reference_wrapper<const Stats::Gauge>>& gauges() override {
    return gauges_;
  };
  const std::vector<std::reference_wrapper<const Stats::ParentHistogram>>& histograms() override {
    return histograms_;
  }

private:
  std::vector<Stats::CounterSharedPtr> snapped_counters_;
  std::vector<CounterSnapshot> counters_;
  std::vector<Stats::GaugeSharedPtr> snapped_gauges_;
  std::vector<std::reference_wrapper<const Stats::Gauge>> gauges_;
  std::vector<Stats::ParentHistogramSharedPtr> snapped_histograms_;
  std::vector<std::reference_wrapper<const Stats::ParentHistogram>> histograms_;
};

} // namespace StrategyHost
} // namespace Envoy