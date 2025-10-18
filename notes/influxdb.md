
```bash
curl -X POST "http://localhost:8181/api/v3/configure/token/admin"  
{"id":0,"name":"_admin","token":"apiv3_Ev09H5kgRiWjyUWqdSCTA51ZGXvokM4hScZGrA69Axd-hmfxDrmN3i-VmR1zhjyIrasO0NsEuPmcWH3mrNbs4Q","hash":"f301559357528ead32fc6ef0661671646e210af4337d1d1b18fad7c76217315335  
45a8f242ad1bb734e501a9005006a3dc0d209c0b2c9d05e8af768778acfd5c","created_at":"2025-10-16T22:23:02.042Z","expiry":null}

curl "http://localhost:8181/health" --header "Authorization: Bearer apiv3_Ev09H5kgRiWjyUWqdSCTA51ZGXvokM4hScZGrA69Axd-hmfxDrmN3i-VmR1zhjyIrasO0NsEuPmcWH3mrNbs4Q"


docker exec -it influxdb /bin/bash
influxdb3 create database --token "apiv3_Ev09H5kgRiWjyUWqdSCTA51ZGXvokM4hScZGrA69Axd-hmfxDrmN3i-VmR1zhjyIrasO0NsEuPmcWH3mrNbs4Q" --retention-period 30d uc
```

grafana:
SQL -> querry language
http://influxdb:8181
token samo je api key sve ostalo iskljucenoi
ukljuceno Insecure Connection