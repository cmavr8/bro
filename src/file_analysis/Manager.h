#ifndef FILE_ANALYSIS_MANAGER_H
#define FILE_ANALYSIS_MANAGER_H

#include <string>
#include <map>
#include <set>

#include "Net.h"
#include "Conn.h"
#include "Val.h"

#include "Info.h"
#include "InfoTimer.h"
#include "FileID.h"

namespace file_analysis {

/**
 * Main entry point for interacting with file analysis.
 */
class Manager {
public:

	Manager();

	~Manager();

	/**
	 * Times out any active file analysis to prepare for shutdown.
	 */
	void Terminate();

	/**
	 * Pass in non-sequential file data.
	 */
	void DataIn(const string& unique, const u_char* data, uint64 len,
	            uint64 offset, Connection* conn = 0,
	            const string& source = "");

	/**
	 * Pass in sequential file data.
	 */
	void DataIn(const string& unique, const u_char* data, uint64 len,
	            Connection* conn = 0, const string& source = "");

	/**
	 * Signal the end of file data.
	 */
	void EndOfFile(const string& unique, Connection* conn = 0,
	               const string& source = "");

	/**
	 * Signal a gap in the file data stream.
	 */
	void Gap(const string& unique, uint64 offset, uint64 len,
	         Connection* conn = 0, const string& source = "");

	/**
	 * Provide the expected number of bytes that comprise a file.
	 */
	void SetSize(const string& unique, uint64 size, Connection* conn = 0,
	             const string& source = "");

	/**
	 * Starts ignoring a file, which will finally be removed from internal
	 * mappings on EOF or TIMEOUT.
	 * @return false if file identifier did not map to anything, else true.
	 */
	bool IgnoreFile(const FileID& file_id);

	/**
	 * If called during \c FileAnalysis::policy evaluation for a
	 * \c FileAnalysis::TRIGGER_TIMEOUT, requests deferral of analysis timeout.
	 */
	bool PostponeTimeout(const FileID& file_id) const;

	/**
	 * Queue attachment of an action to the file identifier.  Multiple actions
	 * of a given type can be attached per file identifier at a time as long as
	 * the arguments differ.
	 * @return false if the action failed to be instantiated, else true.
	 */
	bool AddAction(const FileID& file_id, RecordVal* args) const;

	/**
	 * Queue removal of an action for a given file identifier.
	 * @return true if the action is active at the time of call, else false.
	 */
	bool RemoveAction(const FileID& file_id, const RecordVal* args) const;

	/**
	 * Calls the \c FileAnalysis::policy hook.
	 */
	void EvaluatePolicy(BifEnum::FileAnalysis::Trigger t, Info* info);

protected:

	friend class InfoTimer;

	typedef map<string, Info*> StrMap;
	typedef set<string> StrSet;
	typedef map<FileID, Info*> IDMap;

	/**
	 * @return the Info object mapped to \a unique or a null pointer if analysis
	 *         is being ignored for the associated file.  An Info object may be
	 *         created if a mapping doesn't exist, and if it did exist, the
	 *         activity time is refreshed and connection-related fields of the
	 *         record value may be updated.
	 */
	Info* GetInfo(const string& unique, Connection* conn = 0,
	              const string& source = "");

	/**
	 * @return the Info object mapped to \a file_id, or a null pointer if no
	 *         mapping exists.
	 */
	Info* Lookup(const FileID& file_id) const;

	/**
	 * Evaluate timeout policy for a file and remove the Info object mapped to
	 * \a file_id if needed.
	 */
	void Timeout(const FileID& file_id, bool is_terminating = ::terminating);

	/**
	 * Immediately remove file_analysis::Info object associated with \a unique.
	 * @return false if file string did not map to anything, else true.
	 */
	bool RemoveFile(const string& unique);

	/**
	 * @return whether the file mapped to \a unique is being ignored.
	 */
	bool IsIgnored(const string& unique);

	StrMap str_map; /**< Map unique strings to \c FileAnalysis::Info records. */
	IDMap id_map;   /**< Map file IDs to \c FileAnalysis::Info records. */
	StrSet ignored; /**< Ignored files.  Will be finally removed on EOF. */
};

} // namespace file_analysis

extern file_analysis::Manager* file_mgr;

#endif
