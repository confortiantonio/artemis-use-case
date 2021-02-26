## Use case 4

Project sources:
>tickerplan-prd2 (producer)  
>tickerplan-cns-ticker2 (consumer from  ticker queue)  
>tickerplan-cns-lvq

Run files:
>tickerplan-prd-brk_7.8.0.sh : start producer.
>tickerplan-cns-TICKERPLAN::LVQ.sh : start consumer from LVQ queue

##  Test case 1 LVQ + Retention + Prefetch = 1
Broker configuration:
```
<address-setting match="PUSHSRV.MULTICAST">
  <retroactive-message-count>1000000</retroactive-message-count>
  <default-last-value-key>MESSAGE_KEY</default-last-value-key>
</address-setting>
```

Project sources:
>push-srv-prd
>push-srv-cns

Run files:
>push-srv-prd.sh
>push-srv-cns.sh


## Use case filters using library proton

Project sources:
>push-srv-proton-cns (consumer)
push-srv-proton-prd (producer)  

Run files:
>

## Use case filters using library CMS
Project sources:
>

Run files:
>
