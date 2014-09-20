# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  config.vm.box = "ubuntu/trusty32"

  # Give the vm longer time to boot. Slow machines might want to increase this
  # value (default is 300) if they get stuck at the message:
  # "default: Warning: Connection timeout. Retrying..."
  #config.vm.boot_timeout = 600

  # Set the timezone to the host timezone
  require 'time'
  timezone = 'Etc/GMT' + ((Time.zone_offset(Time.now.zone)/60)/60).to_s
  config.vm.provision :shell, :inline => "if [ $(grep -c UTC /etc/timezone) -gt 0 ]; then echo \"#{timezone}\" | sudo tee /etc/timezone && dpkg-reconfigure --frontend noninteractive tzdata; fi"

  # Fix locale issue as descriped at:
  # http://www.pixelninja.me/how-to-fix-invalid-locale-setting-in-ubuntu-14-04-in-the-cloud/
  config.vm.provision :shell, :inline => 'echo "LC_ALL=\"en_US.UTF-8\"" >> /etc/environment'

  # Run the bootstrap script
  config.vm.provision :shell, path: "bootstrap.sh"


  # We must be able to use the programmer and the usb-serial device
  config.vm.provider :virtualbox do |vb|
    # To get list of usb devices use "VBoxManage list usbhost" and copy the
    # address for the desired device

    # TODO get the correct device addresses
    vb.customize ["usbfilter", "add", "0", "--target", :id,
      "--name", "devices for embedded avr",
      "--action", "hold", # Makes the device available
      "--active", "yes",
    ]
  end

end
