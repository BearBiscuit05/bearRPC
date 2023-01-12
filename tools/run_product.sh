#!/bin/bash
python3 /home/dgl/workspace/launch.py \
--workspace /home/dgl/workspace/ \
--num_trainers 1 \
--num_samplers 0 \
--num_servers 1 \
--part_config data/ogb-product.json \
--ip_config /home/dgl/workspace/ip_config.txt \
"python3 dist_graphSage.py --graph_name ogb-product --ip_config /home/dgl/workspace/ip_config.txt --num_epochs 5 --batch_size 1000 --num_layers 3 --fan_out '10,10,10' --num_hidden 512"