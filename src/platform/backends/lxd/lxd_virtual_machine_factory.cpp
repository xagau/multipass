/*
 * Copyright (C) 2019-2020 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lxd_virtual_machine_factory.h"
#include "lxd_virtual_machine.h"

#include <multipass/format.h>
#include <multipass/logging/log.h>
#include <multipass/utils.h>

#include <QJsonDocument>

namespace mp = multipass;
namespace mpl = multipass::logging;

namespace
{
constexpr auto category = "lxd factory";
} // namespace

mp::LXDVirtualMachineFactory::LXDVirtualMachineFactory(const mp::Path& data_dir, const QUrl& base_url)
    : data_dir{mp::utils::make_dir(data_dir, get_backend_directory_name())},
      base_url{base_url},
      manager{std::make_unique<NetworkAccessManager>()}
{
}

mp::VirtualMachine::UPtr mp::LXDVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                                              VMStatusMonitor& monitor)
{
    return std::make_unique<mp::LXDVirtualMachine>(desc, monitor, manager.get(), base_url);
}

void mp::LXDVirtualMachineFactory::remove_resources_for(const std::string& name)
{
}

mp::FetchType mp::LXDVirtualMachineFactory::fetch_type()
{
    return mp::FetchType::None;
}

mp::VMImage mp::LXDVirtualMachineFactory::prepare_source_image(const mp::VMImage& /* source_image */)
{
    throw std::runtime_error("prepare_source_image unimplemented");
}

void mp::LXDVirtualMachineFactory::prepare_instance_image(const mp::VMImage& /* instance_image */,
                                                          const VirtualMachineDescription& /* desc */)
{
    throw std::runtime_error("prepare_instance_image unimplemented");
}

void mp::LXDVirtualMachineFactory::configure(const std::string& /* name */, YAML::Node& /* meta_config */,
                                             YAML::Node& /* user_config */)
{
}

void mp::LXDVirtualMachineFactory::hypervisor_health_check()
{
    auto reply = lxd_request(manager.get(), "GET", base_url);

    if (reply["metadata"].toObject()["auth"] != QStringLiteral("trusted"))
    {
        mpl::log(mpl::Level::debug, category, "Failed to authenticate to LXD:");
        mpl::log(mpl::Level::debug, category,
                 fmt::format("{}: {}", base_url.toString(), QJsonDocument(reply).toJson(QJsonDocument::Compact)));
        throw std::runtime_error("Failed to authenticate to LXD.");
    }
}
