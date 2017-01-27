# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "ubuntu/xenial64"
  
  config.vm.synced_folder ".", "/vagrant", disabled: false

  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
    v.cpus = 2
  end

  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get install -y cmake wget unzip libexpat1-dev zlib1g-dev libssl-dev libexiv2-dev build-essential devscripts libboost-dev libboost-program-options-dev libboost-timer-dev libboost-filesystem-dev libboost-system-dev

    # Install opencv
    wget https://github.com/Itseez/opencv/archive/3.0.0.zip
    unzip 3.0.0.zip
    sudo chown ubuntu:ubuntu -R opencv-3.0.0/
    cd opencv-3.0.0
    mkdir build
    cd build
    cmake ..
    make
    sudo make install

  SHELL
end
