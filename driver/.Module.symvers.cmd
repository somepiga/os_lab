cmd_/home/hyt/os_lab/driver/Module.symvers := sed 's/\.ko$$/\.o/' /home/hyt/os_lab/driver/modules.order | scripts/mod/modpost -m -a  -o /home/hyt/os_lab/driver/Module.symvers -e    -w -T -
