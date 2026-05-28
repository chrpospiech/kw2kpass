/*
 * wrap_pybind_kp.cpp
 *
 * pybind11 wrapper for libkeepass classes.
 */

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <string>

#include "libkeepass/src/binary.hh"
#include "libkeepass/src/database.hh"
#include "libkeepass/src/entry.hh"
#include "libkeepass/src/exception.hh"
#include "libkeepass/src/group.hh"
#include "libkeepass/src/icon.hh"
#include "libkeepass/src/kdb.hh"
#include "libkeepass/src/kdbx.hh"
#include "libkeepass/src/key.hh"
#include "libkeepass/src/metadata.hh"
#include "libkeepass/src/security.hh"
#include "libkeepass/src/temporal.hh"

namespace py = pybind11;

PYBIND11_MODULE(keepass, m) {
    m.doc() = "Wrapper module for libkeepass";

    py::register_exception<keepass::PasswordError>(m, "PasswordError");
    py::register_exception<keepass::FormatError>(m, "FormatError");
    py::register_exception<keepass::InternalError>(m, "InternalError");
    auto io_error = py::register_exception<keepass::IoError>(m, "IoError");
    py::register_exception<keepass::FileNotFoundError>(m, "FileNotFoundError", io_error.ptr());

    py::class_<keepass::protect<std::string>>(m, "ProtectedString")
        .def(py::init<>())
        .def(py::init<const std::string &, bool>(), py::arg("value"), py::arg("is_protected") = false)
        .def("is_protected", &keepass::protect<std::string>::is_protected)
        .def("set_protected", &keepass::protect<std::string>::set_protected)
        .def("value", &keepass::protect<std::string>::value)
        .def("set_value", &keepass::protect<std::string>::set_value);

    py::class_<keepass::temporal<std::string>>(m, "TemporalString")
        .def(py::init<>())
        .def(py::init<const std::string &, std::time_t>(), py::arg("value"), py::arg("time"))
        .def("value", &keepass::temporal<std::string>::value)
        .def("time", &keepass::temporal<std::string>::time)
        .def("set_time", &keepass::temporal<std::string>::set_time)
        .def("Set", &keepass::temporal<std::string>::Set);

    py::enum_<keepass::Database::Cipher>(m, "Cipher")
        .value("kAes", keepass::Database::Cipher::kAes)
        .value("kTwofish", keepass::Database::Cipher::kTwofish)
        .export_values();

    py::enum_<keepass::Key::SubKeyResolution>(m, "SubKeyResolution")
        .value("kHashSubKeys", keepass::Key::SubKeyResolution::kHashSubKeys)
        .value("kHashSubKeysOnlyIfCompositeKey", keepass::Key::SubKeyResolution::kHashSubKeysOnlyIfCompositeKey)
        .export_values();

    py::class_<keepass::Key>(m, "Key")
        .def(py::init<>())
        .def(py::init<const std::string &>())
        .def("SetPassword", &keepass::Key::SetPassword)
        .def("SetKeyFile", &keepass::Key::SetKeyFile)
        .def("Transform", &keepass::Key::Transform);

    py::class_<keepass::Binary, std::shared_ptr<keepass::Binary>>(m, "Binary")
        .def(py::init<const keepass::protect<std::string> &>())
        .def("Empty", &keepass::Binary::Empty)
        .def("Size", &keepass::Binary::Size)
        .def("data", &keepass::Binary::data)
        .def("set_data", &keepass::Binary::set_data)
        .def("compress", &keepass::Binary::compress)
        .def("set_compress", &keepass::Binary::set_compress);

    py::class_<keepass::Icon, std::shared_ptr<keepass::Icon>>(m, "Icon")
        .def(py::init<const std::array<uint8_t, 16> &, const std::vector<uint8_t> &>())
        .def("uuid", &keepass::Icon::uuid)
        .def("data", &keepass::Icon::data)
        .def("set_data", &keepass::Icon::set_data);

    py::class_<keepass::Entry, std::shared_ptr<keepass::Entry>> entry(m, "Entry");
    py::class_<keepass::Entry::Attachment, std::shared_ptr<keepass::Entry::Attachment>> entry_attachment(entry,
                                                                                                         "Attachment");
    py::class_<keepass::Entry::AutoType> entry_autotype(entry, "AutoType");
    py::class_<keepass::Entry::AutoType::Association> entry_assoc(entry_autotype, "Association");
    py::class_<keepass::Entry::Field> entry_field(entry, "Field");

    entry_attachment.def(py::init<>())
        .def("name", &keepass::Entry::Attachment::name)
        .def("set_name", &keepass::Entry::Attachment::set_name)
        .def("binary", &keepass::Entry::Attachment::binary)
        .def("set_binary", &keepass::Entry::Attachment::set_binary)
        .def("ToJson", &keepass::Entry::Attachment::ToJson);

    entry_assoc.def(py::init<const std::string, const std::string>(), py::arg("window"), py::arg("sequence"))
        .def("window", &keepass::Entry::AutoType::Association::window)
        .def("sequence", &keepass::Entry::AutoType::Association::sequence);

    entry_autotype.def(py::init<>())
        .def("enabled", &keepass::Entry::AutoType::enabled)
        .def("set_enabled", &keepass::Entry::AutoType::set_enabled)
        .def("obfuscation", &keepass::Entry::AutoType::obfuscation)
        .def("set_obfuscation", &keepass::Entry::AutoType::set_obfuscation)
        .def("sequence", &keepass::Entry::AutoType::sequence)
        .def("set_sequence", &keepass::Entry::AutoType::set_sequence)
        .def("associations", &keepass::Entry::AutoType::associations)
        .def("AddAssociation", &keepass::Entry::AutoType::AddAssociation);

    entry_field.def(py::init<const std::string &, const keepass::protect<std::string> &>())
        .def("key", &keepass::Entry::Field::key)
        .def("value", &keepass::Entry::Field::value);

    entry.def(py::init<>())
        .def("uuid", &keepass::Entry::uuid)
        .def("set_uuid", &keepass::Entry::set_uuid)
        .def("icon", &keepass::Entry::icon)
        .def("set_icon", &keepass::Entry::set_icon)
        .def("custom_icon", [](const keepass::Entry &e) { return e.custom_icon().lock(); })
        .def("set_custom_icon", [](keepass::Entry &e, std::shared_ptr<keepass::Icon> icon) { e.set_custom_icon(icon); })
        .def("title", &keepass::Entry::title)
        .def("set_title", &keepass::Entry::set_title)
        .def("url", &keepass::Entry::url)
        .def("set_url", &keepass::Entry::set_url)
        .def("override_url", &keepass::Entry::override_url)
        .def("set_override_url", &keepass::Entry::set_override_url)
        .def("username", &keepass::Entry::username)
        .def("set_username", &keepass::Entry::set_username)
        .def("password", &keepass::Entry::password)
        .def("set_password", &keepass::Entry::set_password)
        .def("notes", &keepass::Entry::notes)
        .def("set_notes", &keepass::Entry::set_notes)
        .def("tags", &keepass::Entry::tags)
        .def("set_tags", &keepass::Entry::set_tags)
        .def("creation_time", &keepass::Entry::creation_time)
        .def("set_creation_time", &keepass::Entry::set_creation_time)
        .def("modification_time", &keepass::Entry::modification_time)
        .def("set_modification_time", &keepass::Entry::set_modification_time)
        .def("access_time", &keepass::Entry::access_time)
        .def("set_access_time", &keepass::Entry::set_access_time)
        .def("expiry_time", &keepass::Entry::expiry_time)
        .def("set_expiry_time", &keepass::Entry::set_expiry_time)
        .def("move_time", &keepass::Entry::move_time)
        .def("set_move_time", &keepass::Entry::set_move_time)
        .def("expires", &keepass::Entry::expires)
        .def("set_expires", &keepass::Entry::set_expires)
        .def("usage_count", &keepass::Entry::usage_count)
        .def("set_usage_count", &keepass::Entry::set_usage_count)
        .def("bg_color", &keepass::Entry::bg_color)
        .def("set_bg_color", &keepass::Entry::set_bg_color)
        .def("fg_color", &keepass::Entry::fg_color)
        .def("set_fg_color", &keepass::Entry::set_fg_color)
        .def("auto_type", &keepass::Entry::auto_type, py::return_value_policy::reference_internal)
        .def("attachments", &keepass::Entry::attachments)
        .def("history", &keepass::Entry::history)
        .def("custom_fields", &keepass::Entry::custom_fields)
        .def("AddAttachment", &keepass::Entry::AddAttachment)
        .def("HasAttachment", &keepass::Entry::HasAttachment)
        .def("AddHistoryEntry", &keepass::Entry::AddHistoryEntry)
        .def("AddCustomField",
             [](keepass::Entry &e, const std::string &key, const keepass::protect<std::string> &value) {
                 std::string mutable_key = key;
                 e.AddCustomField(mutable_key, value);
             })
        .def("HasNonDefaultAutoTypeSettings", &keepass::Entry::HasNonDefaultAutoTypeSettings)
        .def("IsMetaEntry", &keepass::Entry::IsMetaEntry)
        .def("ToJson", &keepass::Entry::ToJson);

    py::class_<keepass::Group, std::shared_ptr<keepass::Group>>(m, "Group")
        .def(py::init<>())
        .def("uuid", &keepass::Group::uuid)
        .def("set_uuid", &keepass::Group::set_uuid)
        .def("icon", &keepass::Group::icon)
        .def("set_icon", &keepass::Group::set_icon)
        .def("custom_icon", [](const keepass::Group &g) { return g.custom_icon().lock(); })
        .def("set_custom_icon", [](keepass::Group &g, std::shared_ptr<keepass::Icon> icon) { g.set_custom_icon(icon); })
        .def("name", &keepass::Group::name)
        .def("set_name", &keepass::Group::set_name)
        .def("notes", &keepass::Group::notes)
        .def("set_notes", &keepass::Group::set_notes)
        .def("creation_time", &keepass::Group::creation_time)
        .def("set_creation_time", &keepass::Group::set_creation_time)
        .def("modification_time", &keepass::Group::modification_time)
        .def("set_modification_time", &keepass::Group::set_modification_time)
        .def("access_time", &keepass::Group::access_time)
        .def("set_access_time", &keepass::Group::set_access_time)
        .def("expiry_time", &keepass::Group::expiry_time)
        .def("set_expiry_time", &keepass::Group::set_expiry_time)
        .def("move_time", &keepass::Group::move_time)
        .def("set_move_time", &keepass::Group::set_move_time)
        .def("flags", &keepass::Group::flags)
        .def("set_flags", &keepass::Group::set_flags)
        .def("expires", &keepass::Group::expires)
        .def("set_expires", &keepass::Group::set_expires)
        .def("expanded", &keepass::Group::expanded)
        .def("set_expanded", &keepass::Group::set_expanded)
        .def("usage_count", &keepass::Group::usage_count)
        .def("set_usage_count", &keepass::Group::set_usage_count)
        .def("default_autotype_sequence", &keepass::Group::default_autotype_sequence)
        .def("set_default_autotype_sequence", &keepass::Group::set_default_autotype_sequence)
        .def("autotype", &keepass::Group::autotype)
        .def("set_autotype", &keepass::Group::set_autotype)
        .def("search", &keepass::Group::search)
        .def("set_search", &keepass::Group::set_search)
        .def("last_visible_entry", [](const keepass::Group &g) { return g.last_visible_entry().lock(); })
        .def("set_last_visible_entry",
             [](keepass::Group &g, std::shared_ptr<keepass::Entry> entry_ptr) { g.set_last_visible_entry(entry_ptr); })
        .def("Groups", &keepass::Group::Groups)
        .def("Entries", &keepass::Group::Entries)
        .def("AddGroup", &keepass::Group::AddGroup)
        .def("AddEntry", &keepass::Group::AddEntry)
        .def("HasNonMetaEntries", &keepass::Group::HasNonMetaEntries)
        .def("ToJson", &keepass::Group::ToJson);

    py::class_<keepass::Metadata, std::shared_ptr<keepass::Metadata>> metadata(m, "Metadata");
    py::class_<keepass::Metadata::MemoryProtection> memory_protection(metadata, "MemoryProtection");
    py::class_<keepass::Metadata::Field> metadata_field(metadata, "Field");

    memory_protection.def(py::init<>())
        .def("title", &keepass::Metadata::MemoryProtection::title)
        .def("set_title", &keepass::Metadata::MemoryProtection::set_title)
        .def("username", &keepass::Metadata::MemoryProtection::username)
        .def("set_username", &keepass::Metadata::MemoryProtection::set_username)
        .def("password", &keepass::Metadata::MemoryProtection::password)
        .def("set_password", &keepass::Metadata::MemoryProtection::set_password)
        .def("url", &keepass::Metadata::MemoryProtection::url)
        .def("set_url", &keepass::Metadata::MemoryProtection::set_url)
        .def("notes", &keepass::Metadata::MemoryProtection::notes)
        .def("set_notes", &keepass::Metadata::MemoryProtection::set_notes);

    metadata_field.def(py::init<const std::string &, const std::string &>())
        .def("key", &keepass::Metadata::Field::key)
        .def("value", &keepass::Metadata::Field::value);

    metadata.def(py::init<>())
        .def("generator", &keepass::Metadata::generator)
        .def("set_generator", &keepass::Metadata::set_generator)
        .def("database_name", &keepass::Metadata::database_name)
        .def("set_database_name", &keepass::Metadata::set_database_name)
        .def("database_desc", &keepass::Metadata::database_desc)
        .def("set_database_desc", &keepass::Metadata::set_database_desc)
        .def("default_username", &keepass::Metadata::default_username)
        .def("set_default_username", &keepass::Metadata::set_default_username)
        .def("maintenance_hist_days", &keepass::Metadata::maintenance_hist_days)
        .def("set_maintenance_hist_days", &keepass::Metadata::set_maintenance_hist_days)
        .def("database_color", &keepass::Metadata::database_color)
        .def("set_database_color", &keepass::Metadata::set_database_color)
        .def("master_key_changed", &keepass::Metadata::master_key_changed)
        .def("set_master_key_changed", &keepass::Metadata::set_master_key_changed)
        .def("master_key_change_rec", &keepass::Metadata::master_key_change_rec)
        .def("set_master_key_change_rec", &keepass::Metadata::set_master_key_change_rec)
        .def("master_key_change_force", &keepass::Metadata::master_key_change_force)
        .def("set_master_key_change_force", &keepass::Metadata::set_master_key_change_force)
        .def("memory_protection", &keepass::Metadata::memory_protection, py::return_value_policy::reference_internal)
        .def("recycle_bin", &keepass::Metadata::recycle_bin)
        .def("set_recycle_bin", &keepass::Metadata::set_recycle_bin)
        .def("recycle_bin_changed", &keepass::Metadata::recycle_bin_changed)
        .def("set_recycle_bin_changed", &keepass::Metadata::set_recycle_bin_changed)
        .def("entry_templates", &keepass::Metadata::entry_templates)
        .def("set_entry_templates", &keepass::Metadata::set_entry_templates)
        .def("entry_templates_changed", &keepass::Metadata::entry_templates_changed)
        .def("set_entry_templates_changed", &keepass::Metadata::set_entry_templates_changed)
        .def("history_max_items", &keepass::Metadata::history_max_items)
        .def("set_history_max_items", &keepass::Metadata::set_history_max_items)
        .def("history_max_size", &keepass::Metadata::history_max_size)
        .def("set_history_max_size", &keepass::Metadata::set_history_max_size)
        .def("last_selected_group", [](const keepass::Metadata &md) { return md.last_selected_group().lock(); })
        .def("set_last_selected_group",
             [](keepass::Metadata &md, std::shared_ptr<keepass::Group> group) { md.set_last_selected_group(group); })
        .def("last_visible_group", [](const keepass::Metadata &md) { return md.last_visible_group().lock(); })
        .def("set_last_visible_group",
             [](keepass::Metadata &md, std::shared_ptr<keepass::Group> group) { md.set_last_visible_group(group); })
        .def("binaries", &keepass::Metadata::binaries)
        .def("icons", &keepass::Metadata::icons)
        .def("fields", &keepass::Metadata::fields)
        .def("AddBinary", &keepass::Metadata::AddBinary)
        .def("AddIcon", &keepass::Metadata::AddIcon)
        .def("AddField", &keepass::Metadata::AddField);

    py::class_<keepass::Database, std::shared_ptr<keepass::Database>>(m, "Database")
        .def(py::init<>())
        .def("root", &keepass::Database::root)
        .def("set_root", &keepass::Database::set_root)
        .def("cipher", &keepass::Database::cipher)
        .def("set_cipher", &keepass::Database::set_cipher)
        .def("master_seed", &keepass::Database::master_seed)
        .def("set_master_seed", py::overload_cast<const std::array<uint8_t, 16> &>(&keepass::Database::set_master_seed))
        .def("set_master_seed", py::overload_cast<const std::vector<uint8_t> &>(&keepass::Database::set_master_seed))
        .def("init_vector", &keepass::Database::init_vector)
        .def("set_init_vector", &keepass::Database::set_init_vector)
        .def("transform_seed", &keepass::Database::transform_seed)
        .def("set_transform_seed", &keepass::Database::set_transform_seed)
        .def("inner_random_stream_key", &keepass::Database::inner_random_stream_key)
        .def("set_inner_random_stream_key", &keepass::Database::set_inner_random_stream_key)
        .def("transform_rounds", &keepass::Database::transform_rounds)
        .def("set_transform_rounds", &keepass::Database::set_transform_rounds)
        .def("compress", &keepass::Database::compress)
        .def("set_compress", &keepass::Database::set_compress)
        .def("meta", &keepass::Database::meta)
        .def("set_meta", &keepass::Database::set_meta);

    py::class_<keepass::KdbFile>(m, "KdbFile")
        .def(py::init<>())
        .def("Import",
             [](keepass::KdbFile &file, const std::string &path, const keepass::Key &key) {
                 std::unique_ptr<keepass::Database> db = file.Import(path, key);
                 return std::shared_ptr<keepass::Database>(db.release());
             })
        .def("Export", &keepass::KdbFile::Export);

    py::class_<keepass::KdbxFile>(m, "KdbxFile")
        .def(py::init<>())
        .def("Import",
             [](keepass::KdbxFile &file, const std::string &path, const keepass::Key &key) {
                 std::unique_ptr<keepass::Database> db = file.Import(path, key);
                 return std::shared_ptr<keepass::Database>(db.release());
             })
        .def("Export", &keepass::KdbxFile::Export);
}
