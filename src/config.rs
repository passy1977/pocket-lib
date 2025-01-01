use crate::pods::device::Device;
use crate::global::DATA_FOLDER;
use std::fs::DirBuilder;
use std::{env, fs};
use std::path::MAIN_SEPARATOR_STR;



#[derive(Debug)]
pub struct Config {
    config_path: Option<String>
}

impl Config {

    pub fn new(config_path: Option<String>) -> Result<Config, &'static str> {

        #[allow(unused)]
        let mut absolute_path = String::new();
        if let Some(path) = config_path {
            absolute_path = path;
        } else {
            absolute_path = env::var("HOME").unwrap();
        }

        if !absolute_path.ends_with(MAIN_SEPARATOR_STR) {
            absolute_path += MAIN_SEPARATOR_STR;
        }
        absolute_path += DATA_FOLDER;

        if !fs::metadata(&absolute_path).is_ok() {
            if !DirBuilder::new().recursive(true).create(&absolute_path).is_ok() {
                return Err("Impossible create folder")
            }
        }

        Ok(Config{
            config_path: Some(absolute_path),
        })
    }


    #[allow(unused)] 
    fn get_config_path(&self) -> &String {
        static ERR_RETURN : String = String::new();

        if let Some(ret) = &self.config_path {
            ret
        } else {
            &ERR_RETURN
        }
    }

    pub fn parse(&self, config_json: &String) -> Result<Device, &'static str> {

        let ret = json::parse(config_json.as_str());
        if let Ok(parsed) = ret {
            let mut device = Device::new();

            if let Some(uuid) = parsed["uuid"].as_str() {
                device.uuid = uuid.to_string();
            } else {
                return Err("Json uuid not find")
            }
            

            if let Some(user_uuid) = parsed["user_uuid"].as_str() {
                device.user_uuid = user_uuid.to_string();
            } else {
                return Err("Json user_uuid not find")
            }

            if let Some(host) = parsed["host"].as_str() {
                device.host = host.to_string();
            } else {
                return Err("Json host not find")
            }

            if let Some(host_pub_key) = parsed["host_pub_key"].as_str() {
                device.host_pub_key = host_pub_key.to_string();
            } else {
                return Err("Json host_pub_key not find")
            }
        
            Ok(device)
        } else {
            return Err("Json parsing error")
        }

    } 
}
