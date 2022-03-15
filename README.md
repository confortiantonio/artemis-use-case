##  Test case  LVQ + Retention + Prefetch = 1

Project sources:
>push-srv-prd  
>push-srv-cns  

Broker configuration:
```
<address-setting match="PUSHSRV.MULTICAST">
  <retroactive-message-count>1000000</retroactive-message-count>
  <default-last-value-key>MESSAGE_KEY</default-last-value-key>
</address-setting>
```

Run files:
>start producer to address PUSHSRV.MULTICAST:  
>./bin-sh/push-srv-prd.sh  

>start consumer from temporary queue  
>./bin-sh/push-srv-cns.sh  


## Test Case 1 - CMS consumer (LVQ / Consumer.PrefetchSize= 1 or 100)

Project sources:
>tickerplan-prd2  
>tickerplan-cns-ticker2 

Broker configuration use case 4:
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
>./bin-sh/tickerplan-prd-brk_7.8.0.sh 

To simulate Test Case 1 (LVQ / Consumer.PrefetchSize = 1)

>start consumer from LVQ queue:
>./bin-sh/tickerplan-cns-TICKERPLAN-LVQ-prefetch-1.sh

To simulate Test Case 1 (LVQ / Consumer.PrefetchSize = 100)
>start consumer from LVQ queue:
>./bin-sh/tickerplan-cns-TICKERPLAN-LVQ-prefetch-100.sh


## Test Case 1 - Proton consumer

Project sources:
>tickerplan-prd2  
>proton-tickerplan-cns 

Broker configuration use case 4:
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
>./bin-sh/tickerplan-prd-brk_7.8.0.sh

>start consumer from LVQ queue:
>./bin-sh/proton-tickerplan-cns.sh



## Use case filters using library qpid-proton-0.33.0

Project sources:
>push-srv-proton-cns (consumer)  
>push-srv-proton-prd (producer)  

Broker configuration
```
<address name="SELECTOR">
            <multicast>
               <queue name="QUEUE.SEL" />
            </multicast>
 </address>
```

Run files:

>start producer :
>./bin-sh/push-srv-proton-prd.sh

>start consumer: 
>./bin-sh/push-srv-proton-cns.sh
