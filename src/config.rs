use crate::pods::device::Device;
use crate::session::Session;
use crate::global::DATA_FOLDER;
use std::fs::DirBuilder;
use std::{env, fs};
use std::path::MAIN_SEPARATOR_STR;



#[derive(Debug)]
pub struct Config {
    config_path: Option<String>,
    pub init: bool
}

impl Config {

    pub fn new(session: &mut Session, config_json: String, config_path: Option<String>) -> Result<Config, &'static str> {

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

        if !fs::metadata(absolute_path.clone()).is_ok() {
            DirBuilder::new().recursive(true).create(absolute_path.clone());
        }

        let ret = json::parse(config_json.as_str());
        if let Ok(parsed) = ret {
            let mut config = Config{
                config_path: Option::None,
                init: false
            };

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

            session.device = Some(device);
            config.config_path = Some(absolute_path);
            config.init = true;
            
            Ok(config)
        } else {
            return Err("Json parsing error")
        }
    }


}
