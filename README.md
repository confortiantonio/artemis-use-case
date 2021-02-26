## Use case 4 

Project sources:
>tickerplan-prd2  
>tickerplan-cns-ticker2 

Broker configuration:
```
<address name="TICKERPLAN">
    <multicast>
       <queue name="LVQ" last-value-key="MESSAGE_KEY" non-destructive="true"/>
       <queue name="TICKER" />
    </multicast>
</address>
```
Run files:
>start producer :
>./bin/tickerplan-prd-brk_7.8.0.sh 

For simulate Test Case 1 (LVQ / Consumer.PrefetchSize = 1)

>start consumer from LVQ queue:
>./bin/tickerplan-cns-TICKERPLAN::LVQ-prefetch-1.sh

For simulate Test Case 1 (LVQ / Consumer.PrefetchSize = 100)
>start consumer from LVQ queue:
>./bin/tickerplan-cns-TICKERPLAN::LVQ-prefetch-1.sh

>start consumer from TICKERqueue:
>./bin/tickerplan-cns-TICKERPLAN::TICKER.sh

##  Test case  LVQ + Retention + Prefetch = 1

Project sources:
>./bin/push-srv-prd  
>./bin/push-srv-cns  

Broker configuration:
```
<address-setting match="PUSHSRV.MULTICAST">
  <retroactive-message-count>1000000</retroactive-message-count>
  <default-last-value-key>MESSAGE_KEY</default-last-value-key>
</address-setting>
```

Run files:
>start producer to address  PUSHSRV.MULTICAST:
>./bin/push-srv-prd.sh 

>start consumer from temporary queue 
>./bin/push-srv-cns.sh


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
