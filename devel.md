In fedora-23 TemplateVM:

sudo yum install gcc sshfs
sudo yum install kernel-devel-1000:4.4.14-11.pvops.qubes

In sys-firewall: Downgrade kernel to 4.4.14-11


NAT settings
-t nat -p udp -d sys-firewall udp dpt:domain to:10.137.1.1
// tcp
-----------------10.137.2.254   --------------- 10.137.1.254
/// tcp
