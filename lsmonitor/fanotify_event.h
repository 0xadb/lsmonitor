#pragma once

#include "spdlog/spdlog.h"

#include "utility.h"

#include <string>
#include <vector>
#include <memory>

#include <sys/types.h>
#include <sys/fanotify.h>

namespace fan
{
  enum class EventCode
  {
    NONE = 0
    , OPEN = FAN_OPEN
    , CLOSE = FAN_CLOSE
  };

  struct FileEvent
  {
    FileEvent(const fanotify_event_metadata * fa)
      : code(
	  (fa->mask & FAN_OPEN ) ? EventCode::OPEN  :
	  (fa->mask & FAN_CLOSE) ? EventCode::CLOSE :
	  EventCode::NONE
	  )
      , pid(fa->pid)
      , uid(-1)
      , gid(getpgid(fa->pid))
      , filename(linux::getFdPath(fa->fd))
      , process(linux::getPidComm(fa->pid))
      , user("n/a")
      , group("n/a"/*linux::getPwgroup(gid)*/) // FIXME: causes recursion on /etc/group
    {
      ++count;
      spdlog::debug("{0}[{1:d}]: start [{2:d}] [{3:d}:{4}] [{5}({6:d}):{7}({8:d})] {9}"
	  , __PRETTY_FUNCTION__
	  , count.load()
	  , static_cast<int>(code)
	  , pid
	  , process.c_str()
	  , user.c_str()
	  , uid
	  , group.c_str()
	  , gid
	  , filename.c_str()
	  );
    }

    FileEvent() = default;
    FileEvent(FileEvent&&) = default;
    FileEvent(const FileEvent&) = default;
    FileEvent& operator=(FileEvent&&) = default;
    FileEvent& operator=(const FileEvent&) = default;
    ~FileEvent()
    {
      --count;
    }

    EventCode code{};
    pid_t pid{};
    uid_t uid{};
    gid_t gid{};
    std::string filename{};
    std::string process{};
    std::string user{};
    std::string group{};

    static std::atomic_int count;
  };
} // lsp