#include "VideoCutter.h"

#include "Messages.h"

#include <Entry.h>
#include <Message.h>
#include <Path.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>
#include <new>
#include <sys/wait.h>
#include <unistd.h>

struct VideoCutter::Job {
	BString source;
	BString output;
	BString start;
	BString duration;
	BMessenger target;
};

BString
VideoCutter::OutputPath(const BString& source)
{
	BPath path(source.String());
	BString leaf(path.Leaf());
	int32 dot = leaf.FindLast('.');
	BString stem = dot > 0 ? BString(leaf.String(), dot) : leaf;
	BString extension = dot > 0 ? BString(leaf.String() + dot) : BString(".mp4");
	BPath parent;
	path.GetParent(&parent);

	for (int32 number = 1; number < 10000; number++) {
		BString suffix;
		if (number > 1)
			suffix.SetToFormat("-%" B_PRId32, number);
		BString candidate;
		candidate.SetToFormat("%s/%s-chop%s%s", parent.Path(), stem.String(),
			suffix.String(), extension.String());
		BEntry entry(candidate.String());
		if (!entry.Exists())
			return candidate;
	}
	return BString();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
VideoCutter::Start(const BString& source, bigtime_t start, bigtime_t end,
	const BMessenger& target, BString& output, BString& error)
{
	output = OutputPath(source);
	if (output.IsEmpty()) {
		error = "Could not choose an output filename.";
		return B_FILE_EXISTS;
	}

	Job* job = new(std::nothrow) Job;
	if (job == nullptr)
		return B_NO_MEMORY;
	job->source = source;
	job->output = output;
	job->start.SetToFormat("%.6f", start / 1000000.0);
	job->duration.SetToFormat("%.6f", (end - start) / 1000000.0);
	job->target = target;

	thread_id thread = spawn_thread(Run, "Kiru video cutter", B_NORMAL_PRIORITY, job);
	if (thread < B_OK) {
		delete job;
		error.SetToFormat("Could not start the cutter (%s).", strerror(thread));
		return thread;
	}
	resume_thread(thread);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
VideoCutter::Run(void* data)
{
	std::unique_ptr<Job> job(static_cast<Job*>(data));
	pid_t child = fork();
	int exitCode = -1;
	if (child == 0) {
		execlp("ffmpeg", "ffmpeg", "-hide_banner", "-loglevel", "error", "-ss",
			job->start.String(), "-i", job->source.String(), "-t",
			job->duration.String(), "-map", "0", "-c", "copy",
			"-avoid_negative_ts", "make_zero", job->output.String(), nullptr);
		_exit(127);
	}
	if (child > 0) {
		int status = 0;
		if (waitpid(child, &status, 0) == child && WIFEXITED(status))
			exitCode = WEXITSTATUS(status);
	}

	BMessage finished(MSG_CUT_FINISHED);
	finished.AddInt32("status", exitCode);
	finished.AddString("output", job->output);
	job->target.SendMessage(&finished);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//
